#include "realTime.h"
#include "RTClib.h"
#include "AVR_Timer1.h"

#define PRINT_TIMEOUT 5  //second
#define UPDATE_CLOCK 3600 //second
#define UPDATE_RTC_TIME 23  //11:00PM 
/********Function prototype*************/
void timerIsr(void);
uint32_t getNtpTime();
void setSecond(uint32_t second);
void printDateTime(DateTime *dtPtr);


/**********Objects global vars**************/
RTC_DS1307 rtc;
uint32_t sec;
uint32_t prevSec;
uint8_t nowHour;
uint8_t prevHour;
tState_t timeState;
DateTime dt;

void realTimeBegin()
{
  timer1.initialize(1);
  timer1.attachIntCompB(timerIsr);
  if (! rtc.begin())
  {
    Serial.println(F("RTC Not Found"));
  }
  if (!rtc.isrunning())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("RTC Adjusted"));
  }
  sec = 0;
  prevSec = 0;
  nowHour = 0;
  prevHour = 0;
  timeState = WAIT;
}

void timerIsr(void)
{
  sec++;
  //  Serial.println(F("Timer ISR Triggered"));
}

uint32_t getNtpTime()
{
  return (1604737235 + 3 * 3600);
}

void printDateTime(DateTime *dtPtr)
{
  char buf4[] = "DD/MM/YYYY-hh:mm:ss";
  Serial.println(dtPtr->toString(buf4));
}
bool  realTimeStart()
{
  uint32_t unixTime = getNtpTime();
  if (unixTime)
  {
    Serial.println(F("Updated RTC & TIMERA"));
    sec = unixTime;            //update processor time
    timer1.start();                 //start processor time

    rtc.adjust(DateTime(unixTime)); //update RTC time
  }
  else
  {
    if (rtc.isrunning())
    {
      Serial.println(F("Updated TIMERA"));
      sec = unixTime;             //update processor time
      timer1.start();             //start processor time
    }
    else
    {
      return false;
    }
  }
  return true;
}

tState_t realTimeSync()
{
  switch (timeState)
  {
    case WAIT:
      if (sec - prevSec >= PRINT_TIMEOUT)
      {
        prevSec = sec;
        timeState = MINUTELY;
      }
      break;
    case MINUTELY:
      Serial.println(F("Minutely Schedule"));
      dt = DateTime(sec);
      printDateTime(&dt);
      nowHour = dt.hour();
//      nowHour = 23;
      if (nowHour > prevHour)
      {
        prevHour = nowHour;
        timeState = HOURLY;
      }
      else
      {
        timeState = WAIT;
      }
      break;
    case HOURLY:
      Serial.println(F("Hourly Schedule"));
      //execute hourly task
      if(nowHour == UPDATE_RTC_TIME)
      {
        timeState = DAILY;
      }
      else
      {
        timeState = WAIT;
      }
      break;
    case DAILY:
      Serial.println(F("Daily Schedule"));
      //execute daily task;
      timeState = WAIT;
      break;
  }
  return timeState;

}




