#include <stdlib.h>
#include <Arduino.h>
#include "current.h"
#include "ACS712.h"

#define CURRENT_PIN 17
//  Arduino UNO has 5.0 volt with a max ADC value of 1023 steps
//  ACS712 5A  uses 185 mV per A
//  ACS712 20A uses 100 mV per A
//  ACS712 30A uses  66 mV per A
// Arduino Example
//ACS712  ACS(A0, 5.0, 1023, 100);
//  ESP 32 example (might requires resistors to step down the logic voltage)
ACS712  ACS(CURRENT_PIN, 3.3, 4095, 100);

/**************************************************************************************************
Function: current_init()
Argument: void
Argument: void 
return: void
**************************************************************************************************/
void current_init(void)
{
  int midpoint;
  int mv_peramp;
  ACS.autoMidPoint();
  ACS.incMidPoint();
  midpoint = ACS.getMidPoint();
  mv_peramp = ACS.getmVperAmp();
  Serial.printf("ACS712 Midpoint: %d\n", midpoint);
  Serial.printf("ACS712 mV per Amp: %d\n", mv_peramp);
}

/**************************************************************************************************
Function: current_get_ma()
Argument: void
Argument: void 
return: current in mA
**************************************************************************************************/
int current_get_ma(void)
{
  int mA = ACS.mA_DC();
  Serial.printf("ACS712: %d mA\n", mA);

  return (mA);
}
