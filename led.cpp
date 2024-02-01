#include <stdlib.h>
#include <Arduino.h>
#include "led.h"
#include <PCA9685.h>

#define DEVICE_COUNT      2
#define DEVICE_FREQUENCY  300


typedef struct LED_STRUCT_TAG
{
  uint16_t Led_AktValue;
  uint16_t Led_LastValue;
}LED_STRUCT;

const uint8_t device_addresses[2] =
{
  0x40,
  0x41
};

#define PCA_DEVICE1 device_addresses[0]  /* first PCA9685 device */
#define PCA_DEVICE2 device_addresses[1]  /* second PCA9685 device */


/*****************************************************************************************/
/*                                         VARIABLES  user                               */
/*****************************************************************************************/
/*                                     0  5 10 15 20  25  30  35  40  45  50  55  60  65  70  75  80  85   90   95      100% */
uint16_t Led_5Percent2Raw_array[21] = {0,1,2,3,5,7,9,13,15,17,19,23,24,26,28,30,33,37,43,50,100};

static LED_STRUCT Led_Struct[LED_MAX]; 

/* Class Constructor */
PCA9685 pca9685;


/**************************************************************************************************
Function: Led_UpdateDriver()
Argument: void
Argument: void 
return: void
**************************************************************************************************/
void Led_UpdateDriver(void)
{
  uint16_t i;
  
  for (i = 0;i < LED_MAX; i++)
  {
    if(Led_Struct[i].Led_AktValue != Led_Struct[i].Led_LastValue)
    {
      if(i <= LED_15)
      {
        /* update driver PWM Driver 1 */
        pca9685.setDeviceChannelDutyCycle(PCA_DEVICE1,i,Led_calc_percent2rawval(Led_Struct[i].Led_AktValue),0);
        //Led_Struct[i].Led_AktValue = faboPWM.get_channel_value(Loc_Address, i);
      }
      else
      {
        /* update driver PWM Driver 2 */
        pca9685.setDeviceChannelDutyCycle(PCA_DEVICE2,i-14,Led_calc_percent2rawval(Led_Struct[i].Led_AktValue),0);
        //Led_Struct[i].Led_AktValue = faboPWM.get_channel_value(Loc_Address, i);
      }
    }
    /* store akt value */
    Led_Struct[i].Led_LastValue = Led_Struct[i].Led_AktValue;
  }
}

/**************************************************************************************************
Function: Led_Init()
Argument: Arg_LedIndex [0...1024][1] ; Index of the LED
return: void
**************************************************************************************************/
void Led_Init()
{
  uint16_t i;

   /* Initialize the pwm servo devices. */
  pca9685.setWire(Wire);
  for (uint8_t Index=0; Index < DEVICE_COUNT; Index++)
  {
    pca9685.addDevice(device_addresses[Index]);
  }
  pca9685.resetAllDevices();
  pca9685.setAllDevicesOutputsNotInverted();
  pca9685.setAllDevicesToFrequency(DEVICE_FREQUENCY);

  Serial.println(F("Init of PWM 12 Bit modules..."));

  /* memset of struct */
  memset(&Led_Struct[0],0x00, LED_MAX * sizeof(&Led_Struct[0]));
 
}

/**************************************************************************************************
Function: Led_SetValue()
Argument: Arg_LedIndex [0...15][1] ; Index of the LED
Argument: Arg_Value [0...4095][1] ; value to set 
return: void
**************************************************************************************************/
void Led_SetValue(LED_INDEX Arg_LedIndex, uint16_t Arg_Value)
{
  if(Arg_LedIndex < LED_MAX)
  {
    Led_Struct[Arg_LedIndex].Led_AktValue = Arg_Value;
  }
}


/**************************************************************************************************
Function: Led_GetValue()
Argument: Arg_LedIndex [0...15][1] ; Index of the LED
return: uint16_t ; value
**************************************************************************************************/
uint16_t Led_GetValue(LED_INDEX Arg_LedIndex)
{
  uint16_t Ret_Value;

  return (Led_Struct[Arg_LedIndex].Led_AktValue);
}

/**************************************************************************************************
Function: Led_calc_percent2rawval()
Argument: Arg_LedIndex [0...18][1] ; Index of the LED
return: uint16_t ; value
**************************************************************************************************/
uint16_t Led_calc_percent2rawval(uint8_t Arg_percent)
{
  uint8_t Loc_Index;
  uint16_t Ret_Value;

 if(Arg_percent == 0)
 {
  Loc_Index = 0;
 }
 else
 {
   Loc_Index = (Arg_percent / 5); 
 } 
 Ret_Value = Led_5Percent2Raw_array[Loc_Index]; 
 
 return (Ret_Value);
}