#ifndef _SYNC_TIME_H
#define _SYNC_TIME_H
#include <Arduino.h>

typedef enum tState_t
{
  WAIT,
  MINUTELY,
  HOURLY,
  DAILY,
};
void realTimeBegin();
bool  realTimeStart();
tState_t realTimeSync();
#endif 
