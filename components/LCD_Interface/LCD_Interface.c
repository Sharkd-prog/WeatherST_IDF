#include <stdio.h>
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <esp_lcd.h>
#include <LCD_Interface.h>
#include <HTTPS_Response.h>

SemaphoreHandle_t SemaphoreHandler;

int Pressed_Button = NO_BUTTON;

void ADC_Task(void* pvParameters){
    adc_oneshot_unit_handle_t ADC1_hand = (adc_oneshot_unit_handle_t) pvParameters;

    int adc_raw_data;
    int Current_Button = NO_BUTTON;
    int Last_Button = NO_BUTTON;
    int Stable_Button = NO_BUTTON;
    unsigned long Last_Debounce_Time = 0;
    const unsigned long Debounce_Delay = 50;
    unsigned long time_tick = 0;

    xSemaphoreGive(SemaphoreHandler);

    while(1){
        adc_oneshot_read(ADC1_hand, ADC_CHANNEL_9, &adc_raw_data);
        if(adc_raw_data < ADC_VAL_BUT_RIG){
            Current_Button = RIGHT_BUTTON;
        }
        else if(adc_raw_data < ADC_VAL_BUT_UP){
            Current_Button = NO_BUTTON;
        }
        else if(adc_raw_data < ADC_VAL_BUT_DOW){
            Current_Button = DOWN_BUTTON;
        }
        else if(adc_raw_data < ADC_VAL_BUT_LEF){
            Current_Button = LEFT_BUTTON;
        }
        else if(adc_raw_data < ADC_VAL_BUT_SEL){
            Current_Button = SELECT_BUTTON;
        }
        else{
            Current_Button = NO_BUTTON;
        }

        time_tick = esp_timer_get_time() / 1000;
        if(Current_Button != Last_Button){
            Last_Debounce_Time = time_tick;
        }
        if((time_tick - Last_Debounce_Time) > Debounce_Delay){
            if(Current_Button != Stable_Button){
                Stable_Button = Current_Button;
                if(Stable_Button != NO_BUTTON){
                    Pressed_Button = Stable_Button;
                    xSemaphoreGive(SemaphoreHandler);
                }
            }
        }
        Last_Button = Current_Button;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void LCD_Task(void* pvParameters){
    lcd_t* LCD = (lcd_t*) pvParameters;

    enum State_Screen Current_Screen = Start_Screen;
    enum Update Draw = Redraw;

    int City_Index = INITIAL_CITY_INDEX;

    char buffer[LENGTH_BUFFER_LCD];

    while(1){
        xSemaphoreTake(SemaphoreHandler, portMAX_DELAY);
        for(int i = 0; i<2; i++){
        switch(Current_Screen)
        {
            case Start_Screen:{
                if(Draw){
                    lcdClear(LCD);
                    lcdSetText(LCD, "Select  city", 2,0);
                    lcdSetText(LCD, "Press SELECT", 2,1);
                    Draw = No_Redraw;
                }

                if(Pressed_Button == SELECT_BUTTON){
                    Current_Screen = Select_Screen;
                    Pressed_Button = NO_BUTTON;
                    Draw = Redraw;
                }
            }
            break;

            case Select_Screen:{
                if(Draw){
                    lcdClear(LCD);
                    lcdSetText(LCD, "City: ", 0,0);
                    lcdSetText(LCD, City[City_Index], 7,0);
                    lcdSetText(LCD, "Count: ", 4,1);
                    sprintf(buffer, "%d", City_Index+1);
                    lcdSetText(LCD, buffer, 11,1);
                    Draw = No_Redraw;
                }

                if(Pressed_Button == SELECT_BUTTON){
                    Current_Screen = Data_Screen;
                    Pressed_Button = NO_BUTTON;
                    Draw = Redraw;
                }

                if(Pressed_Button == RIGHT_BUTTON){
                    City_Index++;
                    if(City_Index >= CITY_COUNT){
                        City_Index = CITY_COUNT-1;
                    }
                    Pressed_Button = NO_BUTTON;
                    Draw = Redraw;
                }

                if(Pressed_Button == LEFT_BUTTON){
                    City_Index--;
                    if(City_Index <= 0){
                        City_Index = 0;
                    }
                    Pressed_Button = NO_BUTTON;
                    Draw = Redraw;
                }

                if(Pressed_Button == DOWN_BUTTON){
                    Current_Screen = Start_Screen;
                    Pressed_Button = NO_BUTTON;
                    Draw = Redraw;
                }
            }
            break;

            case Data_Screen:{
                if(Draw){
                    Get_Weather_Data(City_Index);
                    if(Connect_Status == 1){
                        lcdClear(LCD);
                        lcdSetText(LCD, "Temp", 0,0);
                        lcdSetText(LCD, weather_data[City_Index][TEMP_DATA], 10,0);
                        lcdSetText(LCD, "Humidity", 0,1);
                        lcdSetText(LCD, weather_data[City_Index][HUMIDITY_DATA], 10,1);
                        Draw = No_Redraw;
                    }else{
                        lcdClear(LCD);
                        lcdSetText(LCD, "WiFi is not", 2, 0);
                        lcdSetText(LCD, "connected", 3, 1);
                        Draw = No_Redraw;
                    }
                }
                if(Pressed_Button == SELECT_BUTTON){
                    if(Count_Request_Per_Hour[City_Index] == 0){
                        lcdClear(LCD);
                        lcdSetText(LCD, "No have any data", 0,0);
                    }else{
                        lcdClear(LCD);
                        lcdSetText(LCD, "Temp", 0,0);
                        lcdSetText(LCD, weather_data[City_Index][TEMP_DATA], 10,0);
                        lcdSetText(LCD, "Humidity", 0,1);
                        lcdSetText(LCD, weather_data[City_Index][HUMIDITY_DATA], 10,1);
                        Current_Screen = Select_Screen;
                        Pressed_Button = NO_BUTTON;
                        Draw = No_Redraw;
                    }
                }
                if(Pressed_Button == DOWN_BUTTON){
                    Current_Screen = Select_Screen;
                    Pressed_Button = NO_BUTTON;
                    Draw = Redraw;
                }
            }
        }
    }
}

}

void ADC_Init(){
    adc_oneshot_unit_handle_t ADC1_handler;

    adc_oneshot_unit_init_cfg_t ADC_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&ADC_cfg,&ADC1_handler);

    adc_oneshot_chan_cfg_t cnah_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(ADC1_handler,ADC_CHANNEL_9,&cnah_cfg);

    SemaphoreHandler = xSemaphoreCreateBinary();
    xTaskCreate(ADC_Task, "Task 1", 4095, (void*)ADC1_handler, 5, NULL);
}

void LCD_Init(){

    /* Create LCD object */
    static lcd_t Display;

    /*Init Pin*/
    gpio_num_t data_bus[LCD_DATA_LINE] = {D4_PIN, D5_PIN, D6_PIN, D7_PIN};
    lcdCtor(&Display, data_bus, ENABLE_PIN, REGSEL_PIN);
    lcdInit(&Display);

    /* Clear previous data on LCD */
    lcdClear(&Display);

    xTaskCreate(LCD_Task, "LCD task", 4095, &Display, 4, NULL);

}