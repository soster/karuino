/**
 * Karuino
 * An arduino based clock using an LED matrix display and a real time clock module
 * By Stefan Ostermann 2020
 * BSD-2
 * 
 * */

#include <MD_Parola_lib.h>
#include <MD_Parola.h>

#include <MD_MAX72xx_lib.h>
#include <MD_MAX72xx.h>

#include "MyDS3231.h"



#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

// Wiring:
#define CLK_PIN 9  // or SCK
#define DATA_PIN 8 // or MOSI
#define CS_PIN 10  // or SS

// Button wiring:
// RTC SDA: A4
// RTC SCL: A5
#define MINUS_BUTTON 7
#define PLUS_BUTTON 6
#define MODE_BUTTON 5
#define BRIGHT_BUTTON 4



// Modes:
#define TIME_MODE 0
#define DATE_MODE 1
#define MAX_MODE 1
uint8_t mode = TIME_MODE;

// Button press modes:
#define MODEBUTTON_NONE 0
#define MODEBUTTON_SHORT 1
#define MODEBUTTON_LONG 2

// Modes Settings:
#define SETTINGS_NONE 0
#define SETTINGS_MINUTE 1
#define SETTINGS_HOUR 2
#define MAX_SETTINGS_MODE 2

// Button timings:
#define BUTTON_DELAY 180
// after this, long button press detected:
#define BUTTON_LONG 1000

// Real time clock:
MyDS3231 RTC(0x68);

// LED Display:
MD_Parola mx = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t settingsMode = SETTINGS_NONE;

uint8_t lastSecond = 0;

unsigned long lastTime = 0;

bool refresh = false;

uint8_t intensity = 0;

// the previous state from the input pin
int lastModeButtonState = LOW; 

// the current reading from the input pin
int currentModeButtonState;    

unsigned long modePressedTime = 0;

unsigned long modeReleasedTime = 0;

boolean longPressDetectedWhilePressing = false;

char dateString[11];

char timeString[9];

#define DELAYTIME 100;

void setup()
{
  Serial.begin(115200);
  pinMode(MINUS_BUTTON, INPUT);
  pinMode(PLUS_BUTTON, INPUT);
  pinMode(MODE_BUTTON, INPUT);
  pinMode(BRIGHT_BUTTON, INPUT);

  // Intialize the object:
  mx.begin();
  // Set the intensity (brightness) of the display (0-15):
  mx.setIntensity(intensity);
  // Clear the display:
  mx.displayClear();
}

void loop()
{

  if (mx.displayAnimate() && mode != TIME_MODE)
  {
    mode = TIME_MODE;
  }

  RTC.nowDateTime();

  uint8_t minusButtonState = digitalRead(MINUS_BUTTON);
  uint8_t plusButtonState = digitalRead(PLUS_BUTTON);
  uint8_t brightButtonState = digitalRead(BRIGHT_BUTTON);

  uint16_t button_press_diff = millis() - lastTime;

  bool somethingHappened = false;

  if (minusButtonState == HIGH && button_press_diff > BUTTON_DELAY)
  {
    if (settingsMode==SETTINGS_MINUTE) {
      increaseMinute(1);
    } else if (settingsMode==SETTINGS_HOUR) {
      increaseHour(1);
    }
    somethingHappened = true;
  }

  if (plusButtonState == HIGH && button_press_diff > BUTTON_DELAY)
  {
    if (settingsMode==SETTINGS_MINUTE) {
      increaseMinute(-1);
    } else if (settingsMode==SETTINGS_HOUR) {
      increaseHour(-1);
    }
    somethingHappened = true;
  }

  if (brightButtonState == HIGH && button_press_diff > BUTTON_DELAY)
  {
    intensity += 4;
    if (intensity > 15)
    {
      intensity = 0;
    }
    mx.setIntensity(intensity);
    somethingHappened = true;
  }

  uint8_t modeButtonState = processModeButton();
  if (modeButtonState == MODEBUTTON_LONG)
  {
    if (settingsMode==0) {
      settingsMode=1;
    } else {
      settingsMode=0;
    }
    somethingHappened = true;
  }
  else if (modeButtonState == MODEBUTTON_SHORT)
  {
    if (settingsMode==SETTINGS_NONE) {
      nextMode();
    } else {
      nextSettingsMode();
    }
    
    somethingHappened = true;
  }

  if (somethingHappened)
  {
    lastTime = millis();
    refresh = true;
    RTC.nowDateTime();
  }

  if (refresh || (mode==TIME_MODE && lastSecond != RTC.nowSec)) {
    display();
  }
  

  refresh = false;
  lastSecond = RTC.nowSec;
}

void nextMode()
{
  mode++;
  if (mode > MAX_MODE)
  {
    mode = 0;
  }
}

void nextSettingsMode()
{
  settingsMode++;
  if (settingsMode > MAX_SETTINGS_MODE)
  {
    settingsMode = 1;
  }
}

uint8_t processModeButton()
{
  // read the state of the switch/button:
  currentModeButtonState = digitalRead(MODE_BUTTON);
  uint8_t retval = MODEBUTTON_NONE;
  unsigned long pressDuration;

  if (lastModeButtonState == LOW && currentModeButtonState == HIGH)
  { // button is pressed
    modePressedTime = millis();
    longPressDetectedWhilePressing = false;
  }
  else if (lastModeButtonState == HIGH && currentModeButtonState == LOW && !longPressDetectedWhilePressing)
  { // button is released
    modeReleasedTime = millis();

    pressDuration = modeReleasedTime - modePressedTime;

    if (pressDuration > BUTTON_LONG)
    {
      retval = MODEBUTTON_LONG;
    }
    else
    {
      retval = MODEBUTTON_SHORT;
    }
  }
  else if (lastModeButtonState == HIGH && currentModeButtonState == HIGH && !longPressDetectedWhilePressing)
  { // detect long press while pressing
    pressDuration = millis() - modePressedTime;
    if (pressDuration > BUTTON_LONG)
    {
      retval = MODEBUTTON_LONG;
      longPressDetectedWhilePressing = true;
    }
  }

  // save the the last state
  lastModeButtonState = currentModeButtonState;
  return retval;
}

char* getNumberSettingsText(char* dest,  char* prefix, uint8_t number) {
  dest[0]='\0';
  char numberText[4];
  itoa(number, numberText, 10);
  strcat(dest,prefix);
  strcat(dest,numberText);
  return dest;
}

void display()
{
  if (settingsMode != SETTINGS_NONE)
  {
    switch (settingsMode) {
      case SETTINGS_MINUTE:
      char minString[10];
      getNumberSettingsText(minString,"mm:",RTC.nowMin);
      mx.print(minString);
      break;
      case SETTINGS_HOUR:
      char hourString[10];
      getNumberSettingsText(hourString,"hh:",RTC.nowHour);
      mx.print(hourString);
      break;
    }
  }
  else
  {
    switch (mode)
    {
    case TIME_MODE:
        RTC.getTimeString(timeString, HH_MM);
        mx.setTextAlignment(PA_LEFT);
        mx.print(timeString);
      break;
    case DATE_MODE:
        RTC.getDateString(timeString, false);
        mx.displayScroll(timeString, PA_LEFT, PA_SCROLL_LEFT, 25);
      
      break;
    }
  }
}

void increaseHour(uint8_t incVal)
{
  uint8_t newHour = 0;
  newHour = RTC.nowHour + incVal;
  if (newHour > 23)
  {
    newHour = 0;
  }
  RTC.setTime(newHour, RTC.nowMin, RTC.nowSec);
}

void increaseMinute(uint8_t incVal)
{
  uint8_t newMinute = 0;
  newMinute = RTC.nowMin + incVal;

  if (newMinute > 59)
  {
    newMinute = 0;
  }

  RTC.setTime(RTC.nowHour, newMinute, RTC.nowSec);
}
