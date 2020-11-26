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
#endif 