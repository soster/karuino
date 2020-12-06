//MyDS3231.cpp
//Code fuer Arduino und Attiny45/85
//Author Retian
//Version 2.4


/*
Ansteuerung einer Real Time Clock (RTC) DS3231

MyDS3231 Name(I2C-Adresse);

Beispiel siehe unter:
http://arduino-projekte.webnode.at/meine-libraries/rtc-ds3231/

Funktionen siehe unter:
http://arduino-projekte.webnode.at/meine-libraries/rtc-ds3231/funktionen/

*/

//*************************************************************************
//*************************************************************************

#include "Arduino.h"
#include "MyDS3231.h"

MyDS3231::MyDS3231(byte I2CAdd)
{
  _I2CAdd  = I2CAdd;
  Wire.begin();
}


//*************************************************************************
//DS3231 vorhanden ?

bool MyDS3231::isReady()
{
  Wire.beginTransmission(_I2CAdd);
  if (Wire.endTransmission()== 0) return true;
  else return false;
}


//*************************************************************************
//Auslesen des Datums und der Uhrzeit

void MyDS3231::nowDateTime()
{
  byte inByte;

  Wire.beginTransmission(_I2CAdd);
  Wire.write(DS3231_SECONDS_REG);
  Wire.endTransmission();
  
  Wire.requestFrom(int(_I2CAdd), 7);
  while (Wire.available() == 0);
  for (byte i = 0; i < 7; i++)
  {
    inByte = Wire.read();
    dateTime[i] = bcd2Dec(inByte);
  }

  nowSec = dateTime[0];
  nowMin = dateTime[1];
  nowHour = dateTime[2];
  nowDow = dateTime[3]; //Day of week
  nowDay = dateTime[4];
  nowMonth = dateTime[5];
  nowYear = dateTime[6];
  
}

//*************************************************************************
//Setzen der Uhrzeit

void MyDS3231::setTime(byte h, byte m, byte s)
{
  byte _h = h;
  byte _m = m;
  byte _s = s;
  
  byte time[3];
  
  time[0] = dec2Bcd(_s);
  time[1] = dec2Bcd(_m);
  time[2] = dec2Bcd(_h);
    
  Wire.beginTransmission(_I2CAdd);
  Wire.write(DS3231_SECONDS_REG);
  for (byte i = 0; i < 3; i++)
  {
    Wire.write(time[i]);  
  }
  Wire.endTransmission();
}

//*************************************************************************
//Setzen der Sommerzeit (DST - Daylight Save Time)

void MyDS3231::setDST(bool statusSummerTime)
{
  bool _statusSummerTime = statusSummerTime;	
  byte newHour;
  
  nowDateTime();

  if (_statusSummerTime) 
  {
	if (nowHour == 23) nowHour = 0;
	else nowHour += 1;
  }
  else
  {
	if (nowHour == 0) nowHour = 23;
	else nowHour -= 1;
  }
  newHour = dec2Bcd(nowHour);
  
  Wire.beginTransmission(_I2CAdd);
  Wire.write(DS3231_HOUR_REG);
  Wire.write(newHour);
  Wire.endTransmission();
}

//*************************************************************************
///Setzen des Datums

void MyDS3231::setDate(byte d, byte m, byte y)
{
  byte _y = y;
  byte _m = m;
  byte _d = d;
  
  byte date[3];
  
  date[0] = dec2Bcd(_d);
  date[1] = dec2Bcd(_m);
  date[2] = dec2Bcd(_y);
    
  Wire.beginTransmission(_I2CAdd);
  Wire.write(DS3231_DATE_REG);
  for (byte i = 0; i < 3; i++)
  {
    Wire.write(date[i]);  
  }
  Wire.endTransmission();
}

//*************************************************************************
///Setzen des Wochentages

void MyDS3231::setDow(byte d)
{
  byte _d = d;

  if (_d > 0 && _d <= 7)
  {
    Wire.beginTransmission(_I2CAdd);
    Wire.write(DS3231_DAY_REG);
    Wire.write(_d);  
    Wire.endTransmission();
  }
}

//*************************************************************************
//Lesen der mit "nowDateTime()" eingelesenen Uhrzeit als char-String

void MyDS3231::getTimeString(char* timeString, byte format)
{
  char* _timeString = timeString;
  byte _format = format;
  
  char hourString[3];
  char minString[3];
  char secString[3];
  
  //nowDateTime();
  
  itoa(nowHour, hourString, 10);
  itoa(nowMin, minString, 10);
  itoa(nowSec, secString, 10);
  
  _timeString[0] = '\0';
  if (format == HH_MM_SS)
  {
    if (nowHour < 10) strcat(_timeString, "0");
    strcat(_timeString, hourString);
    strcat(_timeString, ":");
    if (nowMin < 10) strcat(_timeString, "0");
    strcat(_timeString, minString);
    strcat(_timeString, ":");
    if (nowSec < 10) strcat(_timeString, "0");
    strcat(_timeString, secString);
    strcat(_timeString, "\0");
  }
  else if (_format == HH_MM)
  {
    if (nowHour < 10) strcat(_timeString, "0");
    strcat(_timeString, hourString);
    strcat(_timeString, ":");
    if (nowMin < 10) strcat(_timeString, "0");
    strcat(_timeString, minString);
    strcat(_timeString, "\0");
  }
  else if (_format == MM_SS)
  {
    if (nowMin < 10) strcat(_timeString, "0");
    strcat(_timeString, minString);
    strcat(_timeString, ":");
    if (nowSec < 10) strcat(_timeString, "0");
    strcat(_timeString, secString);
    strcat(_timeString, "\0");
  }
}

//*************************************************************************
//Lesen des mit "nowDateTime()" eingelesenen Datums als char-String

void MyDS3231::getDateString(char* dateString, bool format)
{
  char* _dateString = dateString;
  bool _format = format;
	
  char yearString[3];
  char monthString[3];
  char dayString[3];
  
  //nowDateTime();
  
  itoa(nowYear, yearString, 10);
  itoa(nowMonth, monthString, 10);
  itoa(nowDay, dayString, 10);
  
  _dateString[0] = '\0';
  if (nowDay < 10) strcat(_dateString, "0");
  strcat(_dateString, dayString);
  strcat(_dateString, ".");
  if (nowMonth < 10) strcat(_dateString, "0");
  strcat(_dateString, monthString);
  strcat(_dateString, ".");
  if (_format == YEAR_4_DIG) strcat(_dateString, "20");
  if (nowYear < 10) strcat(_dateString, "0");
  strcat(_dateString, yearString);
}

//*************************************************************************
//Lesen des mit "nowDateTime()" eingelesenen Wochentags als char-String

void MyDS3231::getDowString(char* dowString)
{
  char* _dowString = dowString;
  
  _dowString[0] = '\0';
  strcat(_dowString, dayOfWeek[nowDow - 1]);
}


//*************************************************************************
//Umwandlung Dezimal zu BCD und BCD zu Dezimal
//(Interne Verwendung)

uint8_t MyDS3231::dec2Bcd(uint8_t val)
{
  uint8_t _val = val;
  return ((_val / 10 * 16) + (_val % 10));
}

uint8_t MyDS3231::bcd2Dec(uint8_t val)
{
  uint8_t _val = val;
  return ((_val / 16 * 10) + (_val % 16));
}
