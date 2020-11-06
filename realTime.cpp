#include "realTime.h"
#include "RTClib.h"
#include "AVR_Timer1.h"

/********Function prototype*************/
void timerIsr(void);
uint32_t getNtpTime();
void setSecond(uint32_t second);
uint32_t getRtcUnixTime();
void updateRtc(uint32_t unixTime);


/**********Objects global vars**************/
RTC_DS1307 rtc;
uint32_t sec;


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




