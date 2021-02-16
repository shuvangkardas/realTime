#ifndef _SYNC_TIME_H
#define _SYNC_TIME_H
#include <Arduino.h>



void realTimeBegin(funCb_t getntp = NULL);
bool  realTimeStart();
tState_t realTimeSync();
uint32_t second();



void rtBegin(funCb_t getntp = NULL);
bool rtSync(uint32_t uTime);
bool rtsync();
uint32_t ms();

tState_t  rtLoop();


#endif 
