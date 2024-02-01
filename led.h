#ifndef LED_H
#define LED_H

typedef enum
{
  LED_0 = 0,
  LED_1,
  LED_2,
  LED_3,
  LED_4,
  LED_5,
  LED_6,
  LED_7,
  LED_8,
  LED_9,
  LED_10,
  LED_11,
  LED_12,
  LED_13,
  LED_14,
  LED_15,
  LED_16,
  LED_17,
  LED_MAX
}LED_INDEX;

/**************************************************************************************************
Function: Led_Init()
Argument: Arg_LedIndex [0...1024][1] ; Index of the LED
return: void
**************************************************************************************************/
extern void Led_Init(void);

/**************************************************************************************************
Function: Led_UpdateDriver()
Argument: void
Argument: void 
return: void
**************************************************************************************************/
extern void Led_UpdateDriver(void);

/**************************************************************************************************
Function: Led_SetValue()
Argument: Arg_LedIndex [0...15][1] ; Index of the LED
Argument: Arg_Value [0...4095][1] ; value to set 
return: void
**************************************************************************************************/
extern void Led_SetValue(LED_INDEX Arg_LedIndex, uint16_t Arg_Value);

/**************************************************************************************************
Function: Led_GetValue()
Argument: Arg_LedIndex [0...15][1] ; Index of the LED
return: uint16_t ; value
**************************************************************************************************/
extern uint16_t  Led_GetValue(LED_INDEX Arg_LedIndex);

/**************************************************************************************************
Function: Led_calc_percent2rawval()
Argument: Arg_LedIndex [0...18][1] ; Index of the LED
return: uint16_t ; value
**************************************************************************************************/
extern uint16_t Led_calc_percent2rawval(uint8_t Arg_percent);







#endif