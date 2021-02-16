#include "realTime.h"
#include "RTClib.h"
#include "AVR_Timer1.h"

#define PRINT_TIMEOUT 30  //second
#define UPDATE_CLOCK 3600 //second
#define DAILY_SCHEDULE_JOB_TIME 23  //11:00PM 

typedef uint32_t (*timeGetter_t)(void);
typedef void (*timeSetter_t)(uint32_t);
typedef enum tState_t
{
  WAIT,
  MINUTELY,
  HOURLY,
  DAILY,
};
typedef enum RT_SYNC_STATUS_t
{
  NTP_SYNCED,
  RTC_SYNCED,
  UNSYNCED,
};

/************v0.3.0 new api************/


timeGetter_t _rtcGetSec = NULL;
timeSetter_t _rtcUpdateSec = NULL;
timeGetter_t _getNtpTime = NULL;

void rtAttachRTC(timeFun_t setter, timeFun_t getter);
volatile uint32_t _second;
volatile uint32_t _tempSec;
tState_t _timeState;
RT_SYNC_STATUS_t _rtSyncStatus;

void updateSec(uint32_t sec);
void printRtcSyncStatus(RT_SYNC_STATUS_t rtsync);
/********Function prototype*************/
void timerIsr(void);
void setSecond(uint32_t second);
void printDateTime(DateTime *dtPtr);
void startSysTimeFromRtc();
void updateTime(uint32_t NtpTime = 0);

/**********Objects global vars**************/
RTC_DS1307 rtc;
volatile uint32_t sec;

uint32_t prevSec;
uint8_t nowHour;
uint8_t prevHour;

DateTime dt;
funCb_t getNTP;





void timerIsr(void)
{
  _second++;
  _tempSec++;
  Serial.println(F("Timer ISR Triggered"));
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
  return _second;
}

void updateSec(uint32_t sec)
{
  //check if input sec is different from existing sec. 
  _second = sec;
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

void printRtcSyncStatus(RT_SYNC_STATUS_t rtsync)
{
  Serial.print(F("<--Timer sync Status : "))
  switch (rtsync)
  {
  case NTP_SYNCED:
    Serial.print(F("NTP_SYNCED"));
    break;
  case RTC_SYNCED:
    Serial.print(F("RTC_SYNCED"));
    break;
  case UNSYNCED:
     Serial.print(F("UNSYNCED"));
  break;
  }
  Serial.println(F("--->"));
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

void rtAttachRTC(timeGetter_t setter, timeSetter_t getter)
{
  _rtcGetSec = NULL;
  _rtcUpdateSec = NULL;
}

void rtBegin(funCb_t getntp)
{
  _getNtpTime = getntp;
  _second = 0;
  _tempSec = 0;
  _timeState = WAIT;
  _rtSyncStatus = UNSYNCED;
	// prevSec = 0;
	// nowHour = 0;
	// prevHour = 0;
  timer1.initialize(1);
	timer1.attachIntCompB(timerIsr);
}


RT_SYNC_STATUS_t rtSync(uint32_t uTime)
{
	// I have to assume that utime is valid time. 
  if(uTime)
  {
     updateSec(uTime);
     if(_rtcUpdateSec != NULL)
     {
       _rtcUpdateSec(uTime); //this function update rtc time if differs more than 1 sec
     }
     _rtSyncStatus = NTP_SYNCED;
  }
  else
  {
    //when utime is zero, means no time found from rtc
    if(_rtcGetSec != NULL)
    {
      uint32_t rtcSec = _rtcGetSec();
      if(rtcSec)
      {
        //if rtc provides a valid time
        updateSec(rtcSec);
        _rtSyncStatus = RTC_SYNCED;
      }
      else
      {
        //zero means rtc provides an invalid time
        updateSec(0); //start rttimer from zero
        _rtSyncStatus = UNSYNCED;
      }
    }
    else
    {
      // if device has no rtc chip
      updateSec(0);
      _rtSyncStatus = UNSYNCED;
    }
  }
  timer1.start();//start rt timer
  printRtcSyncStatus(_rtSyncStatus);
  return _rtSyncStatus;
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