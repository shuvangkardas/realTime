#ifndef _SYNC_TIME_H
#define _SYNC_TIME_H
#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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


void rtAttachRTC(timeGetter_t getter, timeSetter_t setter);
void rtAttachFastRTC(timeGetter_t getter, timeSetter_t setter,void (*StartRtc)(void));
void rtBegin(timeGetter_t getntp = NULL);
RT_SYNC_STATUS_t rtSync(uint32_t uTime);
RT_SYNC_STATUS_t rtsync();
// uint32_t second();
// uint32_t ms();
tState_t rtLoop();


#endif 
