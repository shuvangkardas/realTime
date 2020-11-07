#ifndef _SYNC_TIME_H
#define _SYNC_TIME_H
#include <Arduino.h>

typedef enum time_event_t
{
  MINUTELY,
  HOURLY,
  DAILY,
};
void realTimeBegin();
bool  realTimeStart();
time_event_t realTimeSync();
#endif 
