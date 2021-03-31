#include "realTime.h"
#if defined(PROD_BUILD)
  #include "../../RTClib/RTClib.h"
  // #include "../../Timer1/src/AVR_Timer1.h"
#else
  #include "RTClib.h"
  // #include "AVR_Timer1.h"
#endif
#define PRINT_TIMEOUT 30  //second
#define UPDATE_CLOCK 3600 //second
#define DAILY_SCHEDULE_JOB_TIME 23  //11:00PM 

/********Function prototype*************/
// void timerIsr(void);
// void updateSec(uint32_t sec);
void printDateTime(DateTime *dtPtr);

// void rtAttachRTC(timeSetter_t setter, timeGetter_t getter);
void printRtcSyncStatus(RT_SYNC_STATUS_t rtsync);

/***********Global Vars****************/
timeGetter_t _rtcGetSec = NULL;
timeSetter_t _rtcUpdateSec = NULL;

timeGetter_t _getNtpTime = NULL;

timeGetter_t second = NULL;
timeSetter_t updateSec = NULL;
void (*startFastRtc)(void) = NULL;

// volatile uint32_t _second;
// volatile uint32_t _tempSec;
uint32_t _prevSecond;
uint8_t _nowHour;
uint8_t _prevHour;

tState_t _timeState;
RT_SYNC_STATUS_t _rtSyncStatus;
DateTime _dt;


void rtBegin(timeGetter_t getntp)
{
  _getNtpTime = getntp;
  _prevSecond = 0;

  _timeState = WAIT;
  _rtSyncStatus = UNSYNCED;
  _nowHour = 0;
  _prevHour = 0;
}

void rtAttachRTC( timeGetter_t getter, timeSetter_t setter)
{
  _rtcGetSec = getter;
  _rtcUpdateSec = setter;
}

void rtAttachFastRTC(timeGetter_t getter, timeSetter_t setter,void (*StartRtc)(void))
{
  second = getter;
  updateSec = setter;
  startFastRtc = StartRtc;
}




RT_SYNC_STATUS_t rtSync(uint32_t uTime)
{
	// I have to assume that utime is valid time. 
  if(uTime)
  {
     if(updateSec !=NULL)
     {
        updateSec(uTime);
     }

     if(_rtcUpdateSec != NULL)
     {
       _rtcUpdateSec(uTime); //this function update rtc time if differs more than 1 sec
     }
     _rtSyncStatus = NTP_SYNCED;
  }
  else
  {
    //when utime is zero, means no time found from ntp
    if(_rtcGetSec != NULL)
    {
      uint32_t rtcSec = _rtcGetSec();
      if(rtcSec)
      {
        //if rtc provides a valid time
        if(updateSec !=NULL)
        {
          updateSec(rtcSec);
        }
        _rtSyncStatus = RTC_SYNCED;
      }
      else
      {
        //zero means rtc provides an invalid time
        if(updateSec !=NULL)
        {
          updateSec(0); //start rttimer from zero
        }
        _rtSyncStatus = UNSYNCED;
        Serial.println(F("Unsynced and RTC Failed"));
      }
    }
    else
    {
      // if device has no rtc chip
      if(updateSec !=NULL)
      {
        updateSec(0);
      }
      _rtSyncStatus = UNSYNCED;
    }
  }
  // timer1.start();//start rt timer
  if(startFastRtc != NULL)
  {
    startFastRtc();
  }
  printRtcSyncStatus(_rtSyncStatus);

  _dt = DateTime(second());
  _nowHour = _dt.hour();
  _prevHour = _nowHour;
  return _rtSyncStatus;
}


RT_SYNC_STATUS_t rtsync()
{
  RT_SYNC_STATUS_t syncStatus = UNSYNCED;
	if(_getNtpTime)
	{
		uint32_t ntpUnix = _getNtpTime();
		syncStatus = rtSync(ntpUnix);
	}
  return syncStatus;
}

tState_t rtLoop()
{
  switch (_timeState)
  {
    case WAIT:
      if (second() - _prevSecond >= PRINT_TIMEOUT)
      {
        _prevSecond = second();
        _timeState = MINUTELY;
      }
      break;
    case MINUTELY:
      // Serial.println(F("Minutely Schedule"));
      _dt = DateTime(second());
      printDateTime(&_dt);
      _nowHour = _dt.hour();
      //      nowHour = 23;
      if (_nowHour > _prevHour)
      {
        _prevHour = _nowHour;
        _timeState = HOURLY;
      }
      else
      {
        _timeState = WAIT;
      }
      break;
    case HOURLY:
      Serial.println(F("Hourly Schedule"));
      //execute hourly task
      if (_nowHour == DAILY_SCHEDULE_JOB_TIME)
      {
        _timeState = DAILY;
      }
      else
      {
        _timeState = WAIT;
      }
      break;
    case DAILY:
      Serial.println(F("Daily Schedule"));
      //execute daily task;
      _timeState = WAIT;
      break;
  }
  return _timeState;
}

void printDateTime(DateTime *dtPtr)
{
  char buf4[] = "DD/MM/YYYY-hh:mm:ss";
  Serial.print(F("|------------------------------------|\r\n|         "));
  Serial.print(dtPtr->toString(buf4));
  Serial.println(F("        |\r\n|------------------------------------|"));
}


void printRtcSyncStatus(RT_SYNC_STATUS_t rtsync)
{
  Serial.print(F("<--Timer sync Status : "));
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