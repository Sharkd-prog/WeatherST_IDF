#pragma once

#define D4_PIN     GPIO_NUM_4
#define D5_PIN     GPIO_NUM_5
#define D6_PIN     GPIO_NUM_6
#define D7_PIN     GPIO_NUM_7
#define ENABLE_PIN GPIO_NUM_17
#define REGSEL_PIN GPIO_NUM_18

#define RIGHT_BUTTON  0
#define LEFT_BUTTON   1
#define SELECT_BUTTON 2
#define DOWN_BUTTON   3
#define NO_BUTTON     4

#define ADC_VAL_BUT_RIG 200
#define ADC_VAL_BUT_UP  1000
#define ADC_VAL_BUT_DOW 1500
#define ADC_VAL_BUT_LEF 2050
#define ADC_VAL_BUT_SEL 3000

#define LENGTH_BUFFER_LCD 15

enum State_Screen{
    Start_Screen,           
    Select_Screen,
    Data_Screen
};

#define INITIAL_CITY_INDEX 0

enum Update{
    No_Redraw,
    Redraw
};

void LCD_Init();
void ADC_Init();