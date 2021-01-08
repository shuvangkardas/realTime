#include "realTime.h"
#include "RTClib.h"
#include "AVR_Timer1.h"

#define PRINT_TIMEOUT 30  //second
#define UPDATE_CLOCK 3600 //second
#define DAILY_SCHEDULE_JOB_TIME 23  //11:00PM 
/********Function prototype*************/
void timerIsr(void);
void setSecond(uint32_t second);
void printDateTime(DateTime *dtPtr);
uint32_t getRtcUnix();
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
  uint32_t temp = (uint32_t)tcnt1*(uint32_t)1000;
  // Serial.println(temp);
  temp = temp/(uint32_t)ICR1;
  // Serial.println(temp);

  temp = _tempSec*(uint32_t)1000 + temp;
  return temp;
}

uint32_t second()
{
  return sec;
}

uint32_t getRtcUnix()
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
  uint32_t rtcUnix = getRtcUnix();
  sec = rtcUnix;             //update processor time
  timer1.start();             //start processor time
}

void updateTime(uint32_t NtpTime)
{
  if (NtpTime)
  {
    Serial.println(F("Updated from NTP"));
    sec = NtpTime;
    timer1.start();
    rtc.adjust(DateTime(sec));
  }
  else
  {
    Serial.println(F("Updated from RTC"));
    uint32_t rtcUnix = getRtcUnix();
    sec = rtcUnix;             //update processor time
    timer1.start();             //start processor time
  }
}



bool  realTimeStart()
{
  bool rtcRunning = rtc.isrunning();
  if (getNTP != NULL)
  {
    uint32_t ntpUnix = getNTP();
    if(ntpUnix)
    {
      updateTime(ntpUnix);
      Serial.println(F("Updated RTC & TIMERA"));
    }
    else
    {
      if(rtcRunning)
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
	getNTP = getntp;
}

bool rtSync(uint32_t uTime)
{
	
	if(uTime)
	{
	  updateTime(uTime);
      Serial.println(F("Updated RTC & TIMERA"));
	}
	else
	{
		bool rtcRunning = rtc.isrunning();
		if(rtcRunning)
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


bool rtsync()
{
	if(getNTP)
	{
		uint32_t ntpUnix = getNTP();
		bool ok = rtSync(ntpUnix);
		return ok;
	}
	else
	{
		return false;
	}
}