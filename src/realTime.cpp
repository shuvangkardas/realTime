#include "realTime.h"
#include "RTClib.h"
#include "AVR_Timer1.h"

#define PRINT_TIMEOUT 30           //second
#define UPDATE_CLOCK 3600          //second
#define DAILY_SCHEDULE_JOB_TIME 23 //11:00PM
#define ONE_HOUR_SEC UPDATE_CLOCK
/********Function prototype*************/
void timerIsr(void);
void setSecond(uint32_t second);
void printDateTime(DateTime *dtPtr);

void startSysTimeFromRtc();
void updateTime(uint32_t NtpTime = 0);

/**********Objects global vars**************/
RTC_DS1307 rtc;
volatile uint32_t sec;
volatile uint32_t _tempSec;
uint32_t prevSec;
uint8_t nowHour;
uint8_t prevHour;
tState_t timeState;
DateTime dt;
funCb_t getNTP;

void realTimeBegin(funCb_t getntp)
{
  timer1.initialize(1);
  timer1.attachIntCompB(timerIsr);
  if (!rtc.begin())
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
  getNTP = getntp;
}

void timerIsr(void)
{
  sec++;
  _tempSec++;
  //  Serial.println(F("Timer ISR Triggered"));
}

uint32_t ms()
{
  // 1000*TCNT1/ICR1 + second*1000;
  uint16_t tcnt1 = TCNT1;
  // Serial.println(F("------"));
  // Serial.println(tcnt1);
  // Serial.println(ICR1);
  uint32_t temp = (uint32_t)tcnt1 * (uint32_t)1000;
  // Serial.println(temp);
  temp = temp / (uint32_t)ICR1;
  // Serial.println(temp);

  temp = _tempSec * (uint32_t)1000 + temp;
  return temp;
}

uint32_t second()
{
  return sec;
}

uint32_t rtcUnix()
{
  DateTime now = rtc.now();
  return now.unixtime();
}

void printDateTime(DateTime *dtPtr)
{
  char buf4[] = "DD/MM/YYYY-hh:mm:ss";
  Serial.print(F("|------------------------------------|\r\n|         "));
  Serial.print(dtPtr->toString(buf4));
  Serial.println(F("        |\r\n|------------------------------------|"));
}
void startSysTimeFromRtc()
{
  Serial.println(F("Processor time updated from RTC"));
  uint32_t rtcunix = rtcUnix();
  sec = rtcunix;  //update processor time
  timer1.start(); //start processor time
}

void updateTime(uint32_t NtpTime)
{
  if (NtpTime)
  {
    Serial.println(F("All clock Updated from NTP"));
    sec = NtpTime;
    timer1.start();
    rtc.adjust(DateTime(sec));
  }
  else
  {
    Serial.println(F("Sys Clock updated from RTC"));
    uint32_t rtcunix = rtcUnix();
    sec = rtcunix;  //update processor time
    timer1.start(); //start processor time
  }
}

bool realTimeStart()
{
  bool rtcRunning = rtc.isrunning();
  if (getNTP != NULL)
  {
    uint32_t ntpUnix = getNTP();
    if (ntpUnix)
    {
      updateTime(ntpUnix);
      Serial.println(F("Updated RTC & TIMERA"));
    }
    else
    {
      if (rtcRunning)
      {
        updateTime();
        Serial.println(F("NTP FAILED, TIMER UPDATED"));
      }
      else
      {
        return false;
      }
    }
  }
  else
  {
    if (rtcRunning)
    {
      updateTime();
      Serial.println(F("NTP UNAVAILAVLE, TIMER UPDATED"));
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
    // Serial.println(F("Minutely Schedule"));
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
    if (nowHour == DAILY_SCHEDULE_JOB_TIME)
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

/****************New Library******************/
void rtBegin(funCb_t getntp)
{
  sec = 0;
  prevSec = 0;
  nowHour = 0;
  prevHour = 0;
  timeState = WAIT;
  getNTP = getntp;

  timer1.initialize(1);
  timer1.attachIntCompB(timerIsr);
  if (!rtc.begin())
  {
    Serial.println(F("RTC Not Found"));
  }
  if (!rtc.isrunning())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("RTC Adjusted"));
  }
  else
  {
    startSysTimeFromRtc();
    // uint32_t rtcunix = rtcUnix();
    // sec = rtcunix;  //update processor time
    // timer1.start(); //start processor time
    // Serial.println(F("Sys time start from rtc"));
    // updateTime();
  }
}

bool rtSync(uint32_t uTime)
{

  if (uTime)
  {
    updateTime(uTime);
    Serial.println(F("Updated RTC & TIMERA"));
  }
  else
  {
    bool rtcRunning = rtc.isrunning();
    if (rtcRunning)
    {
      updateTime();
      Serial.println(F("NTP FAILED, TIMER UPDATED"));
    }
    else
    {
      return false;
    }
  }
  return true;
}

bool rtSync2(uint32_t uTime)
{

  uint32_t rtcunix = rtcUnix();
  int32_t timeDiff = uTime - rtcunix;

  Serial.print(F("Utime : "));
  Serial.println(uTime);
  Serial.print(F("rtcunix : "));
  Serial.println(rtcunix);
  Serial.print(F("timediff : "));
  Serial.println(timeDiff);
  timeDiff = abs(timeDiff);
  if (timeDiff <= ONE_HOUR_SEC)
  {
    Serial.println(F("Time is close to rtc"));
    sec = uTime;
    timer1.start();
    if (timeDiff > 0)
    {
      //if time shifts then change time, else ignore.
      rtc.adjust(DateTime(sec));
      Serial.println(F("RTC updated from NTP"));
    }
  }
  else
  {
    bool rtcRunning = rtc.isrunning();
    if (rtcRunning)
    {
      startSysTimeFromRtc();
      // Serial.println(F("Processor time updated from RTC"));
      // uint32_t rtcunix = rtcUnix();
      // sec = rtcunix;  //update processor time
      // timer1.start(); //start processor time
      // updateTime();
    }
    else
    {
      Serial.println(F("Device lost time :( "));
    }
  }
  return true;
}

bool rtsync()
{
  if (getNTP)
  {
    uint32_t ntpUnix = getNTP();
    bool ok = rtSync2(ntpUnix);
    return ok;
  }
  else
  {
    return false;
  }
}