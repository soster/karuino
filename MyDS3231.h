//MyDS3231

#ifndef MyDS3231_h
#define MyDS3231_h

#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__) || defined (__AVR_ATtiny84__)
  #include "TinyWireM.h"
  #define Wire TinyWireM
#else
  #include <Wire.h>
#endif

#include "Arduino.h"

//Register des DS3231
#define DS3231_SECONDS_REG 0x00
#define DS3231_MINUTES_REG 0x01
#define DS3231_HOUR_REG    0x02
#define DS3231_DAY_REG     0x03
#define DS3231_DATE_REG    0x04
#define DS3231_MONTH_REG   0x05
#define DS3231_YEAR_REG    0x06	

#define YEAR_2_DIG  0
#define YEAR_4_DIG  1
#define HH_MM_SS    0
#define HH_MM       1
#define MM_SS       2
#define DST_OFF     0
#define DST_ON      1

class MyDS3231
{
  public:
  MyDS3231(byte);
  bool isReady(void);
  void nowDateTime(void);
  void setTime(byte, byte, byte);
  void setDST(bool); //Sommerzeit setzen (Daylight Save Time)
  void setDate(byte, byte, byte);
  void setDow(byte);
  void getTimeString(char*, byte);
  void getDateString(char*, bool);
  void getDowString(char*);
	
  uint8_t nowSec;
  uint8_t nowMin;
  uint8_t nowHour;
  uint8_t nowDay; 
  uint8_t nowMonth;
  uint8_t nowYear;
  uint8_t nowDow; //day of week
							 
        
  private:
  uint8_t dec2Bcd(uint8_t);
  uint8_t bcd2Dec(uint8_t);

  byte _I2CAdd;
  uint8_t dateTime[7];
  const char* dayOfWeek[7] = {
   "Mo", "Di", "Mi",
   "Do", "Fr", "Sa", "So"};
};

#endif
