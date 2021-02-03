#ifndef _SYNC_TIME_H
#define _SYNC_TIME_H
#include <Arduino.h>

typedef uint32_t (*funCb_t)(void);


typedef enum tState_t
{
  WAIT,
  MINUTELY,
  HOURLY,
  DAILY,
};
void realTimeBegin(funCb_t getntp = NULL);
bool  realTimeStart();
tState_t realTimeSync();
uint32_t second();


uint32_t rtcUnix();
void rtBegin(funCb_t getntp = NULL);
bool rtSync(uint32_t uTime);
bool rtSync2(uint32_t uTime);
bool rtsync();
uint32_t ms();

tState_t  rtLoop();


#endif 
