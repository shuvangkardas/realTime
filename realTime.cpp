#include "realTime.h"
#include "RTClib.h"
#include "AVR_Timer1.h"

#define PRINT_TIMEOUT 60  //second
#define UPDATE_CLOCK 3600 //second
#define UPDATE_RTC_TIME 23  //11:00PM 
/********Function prototype*************/
void timerIsr(void);
uint32_t getNtpTime();
void setSecond(uint32_t second);
uint32_t getRtcUnixTime();
void updateRtc(uint32_t unixTime);
bool dailyUpdateflag;

/**********Objects global vars**************/
RTC_DS1307 rtc;
uint32_t sec;
uint32_t prevSec;
uint8_t prevHour;

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

  prevSec = 0;
  sec = 0;
  dailyUpdateflag = false;
}

void timerIsr(void)
{
  sec++;
  Serial.println(F("Timer ISR Triggered"));
}

uint32_t getRtcUnixTime()
{
  DateTime now = rtc.now();
  return now.unixtime();
}

uint32_t getNtpTime()
{
  return 0;
}

void updateRtc(uint32_t unixTime)
{
  rtc.adjust(DateTime(unixTime));
}

bool  realTimeStart()
{
  uint32_t unixTime = getNtpTime();
  if (unixTime)
  {
    setSecond(unixTime);            //update processor time
    timer1.start();                 //start processor time
    
    rtc.adjust(DateTime(unixTime)); //update RTC time
  }
  else
  {
    if (rtc.isrunning())
    {
      setSecond(unixTime);            //update processor time
      timer1.start();                 //start processor time
    }
    else
    {
      return false;
    }
  }
  return true;
}

void realTimeSync()
{
  if(sec - prevSec >= PRINT_TIMEOUT)
  {
    DateTime dt(sec);
    char buf4[] = "DD/MM/YYYY-hh:mm:ss";
    Serial.println(dt.toString(buf4));

    uint8_t nowHour = dt.hour();
    if(nowHour>prevHour)
    {
      prevHour = nowHour;
      //execute hourly schedule job
      if(nowHour == UPDATE_RTC_TI && dailyUpdateflag == false)
      {
        //execute daily schedule job
        dailyUpdateflag == true;
      }
      else
      {
        dailyUpdateflag = false;
      }
    }
  }
}




