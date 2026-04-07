idf_component_register(SRCS "LCD_Interface.c" 
                    INCLUDE_DIRS "include"
                    REQUIRES esp_adc esp_timer HTTPS_Response ESP32-LCD-1602-Driver)