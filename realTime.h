#ifndef _SYNC_TIME_H
#define _SYNC_TIME_H
#include <Arduino.h>

typedef enum time_state_t
{
  WAIT,
  MINUTELY,
  HOURLY,
  DAILY,
};
void realTimeBegin();
bool  realTimeStart();
time_state_t realTimeSync();
#endif 
