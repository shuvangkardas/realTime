#ifndef _SYNC_TIME_H
#define _SYNC_TIME_H
#include <Arduino.h>

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

void realTimeBegin(funCb_t getntp = NULL);
bool  realTimeStart();
tState_t realTimeSync();
uint32_t second();



void rtBegin(funCb_t getntp = NULL);
RT_SYNC_STATUS_t rtSync(uint32_t uTime);
bool rtsync();
uint32_t ms();

tState_t rtLoop();


#endif 
