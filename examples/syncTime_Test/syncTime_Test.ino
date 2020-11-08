#include "realTime.h"

uint32_t getNtpTime();
void setup()
{
  Serial.begin(9600);
  realTimeBegin(getNtpTime);
  //  realTimeBegin();
  realTimeStart();
  Serial.println(F("Setup Done"));

}

void loop()
{
  realTimeSync();
}

uint32_t getNtpTime()
{
//  return 0;
  return (1604819998+6*3600);
  //  return (1604737235 + 3 * 3600);
}
