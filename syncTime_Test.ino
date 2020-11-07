#include "realTime.h"
void setup() 
{
  Serial.begin(9600);
  realTimeBegin();
  realTimeStart();
  Serial.println(F("Setup Done"));
  
}

void loop() 
{
  realTimeSync();
}
