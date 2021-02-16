#include "realTime.h"
#include "RTClib.h"
uint32_t getNtpTime();
void rtcBegin();
uint32_t rtcGetSec();
void rtcUpdateSec(uint32_t unix);

RTC_DS1307 rtc;
void setup()
{
  Serial.begin(9600);
  rtcBegin();
  rtAttachRTC(rtcGetSec, rtcUpdateSec);
  rtBegin(getNtpTime);
  rtsync();
  Serial.println(F("Setup Done"));

}

void loop()
{
  rtLoop();
  //  Serial.print(F("time : "));Serial.println(rtcGetSec());
}

void rtcBegin()
{
  if (! rtc.begin())
  {
    Serial.println(F("RTC Not Found"));
  }
  else
  {
    Serial.println(F("RTC Found"));
  }
  if (!rtc.isrunning())
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("RTC Adjusted"));
  }
}

uint32_t rtcGetSec()
{
  uint32_t utime = 0;
  if (rtc.isrunning())
  {
    Serial.println(F("Getting RTC time"));
    DateTime now = rtc.now();
    utime =  now.unixtime();
  }
  return utime;
//  return 0;
}
void rtcUpdateSec(uint32_t unix)
{
  if (rtc.isrunning())
  {
    Serial.println(F("Setting RTC time"));
    rtc.adjust(DateTime(unix));
  }
}

uint32_t getNtpTime()
{
//      return 0;
  return (1604819998 + 6 * 3600);
  //  return (1604737235 + 3 * 3600);
}
