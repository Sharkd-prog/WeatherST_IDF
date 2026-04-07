#include <stdio.h>
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_crt_bundle.h"
#include <cJSON.h>
#include "wifi_manage.h"
#include <HTTPS_Response.h>

char weather_data[CITY_COUNT][WEATHER_DATA_COUNT][LENGTH_BUFFER];
const char* City[CITY_COUNT]={"Kyiv", "Lviv"};
const char* Contry_code[CITY_COUNT]= {"UA", "UA"};
const char* Api_key = "3da792c19c51572a5d6ed31d066561f9";
int Count_Request_Per_Hour[CITY_COUNT_PERMITTED_REQUEST] = {0};

esp_http_client_handle_t client = INIT_CLIENT;

int Connect_Status;

esp_err_t https_get_handler(esp_http_client_event_t *evt){
    
    switch(evt->event_id){
        case HTTP_EVENT_ON_DATA:
        int Index = (int)evt->user_data;
        //int val = Wait_connect_to_wifi();
        //if(val == WiFi_CONNECTED){
          //  Connect_Status = 1;
            if(Count_Request_Per_Hour[Index] == 0){
                cJSON* JSON_Data = cJSON_Parse(evt->data);
                cJSON* JSON_Data_main = cJSON_GetObjectItem(JSON_Data, "main");

                cJSON* temp = cJSON_GetObjectItem(JSON_Data_main, "temp");
                sprintf(weather_data[Index][TEMP_DATA],"%.1f",temp->valuedouble);

                cJSON* humid = cJSON_GetObjectItem(JSON_Data_main, "humidity");
                sprintf(weather_data[Index][HUMIDITY_DATA],"%d",humid->valueint);
                cJSON_Delete(JSON_Data);
                Count_Request_Per_Hour[Index] = 1;
                break;
            }
        //}else{
        //    Connect_Status = 0;
        //}
        default:
        break;
    }
    return ESP_OK;
}

void Permission_Refresh_Data(TimerHandle_t Timer){
    printf("Timer finished count \n");
    for(int i = 0; i < CITY_COUNT_PERMITTED_REQUEST; i++){
        Count_Request_Per_Hour[i] = 0;
    }
}

void Get_Weather_Data(int Index_City){
    if(client == INIT_CLIENT){
        esp_http_client_config_t cfg = {
            .url = "https://api.openweathermap.org",
            .method = HTTP_METHOD_GET,
            .event_handler = https_get_handler,
            .auth_type = HTTP_AUTH_TYPE_NONE,
            .transport_type = HTTP_TRANSPORT_OVER_SSL,
            .timeout_ms = 5000,
            .buffer_size = 1024,
            .crt_bundle_attach = esp_crt_bundle_attach
        };
        client = esp_http_client_init(&cfg);
    }
    int Connect = Wait_connect_to_wifi();
    if(Connect == WiFi_CONNECTED){
        Connect_Status = 1;
        char url[150] = "https://api.openweathermap.org/data/2.5/weather?q=";
        sprintf(url + strlen(url),"%s",City[Index_City]);
        sprintf(url + strlen(url),"%s",",");
        sprintf(url + strlen(url),"%s",Contry_code[Index_City]);
        sprintf(url + strlen(url),"%s","&appid=");
        sprintf(url + strlen(url),"%s",Api_key);
        sprintf(url + strlen(url),"%s","&units=metric");
        esp_http_client_set_url(client, url);
        esp_http_client_set_user_data(client, (void*)Index_City);
        esp_http_client_perform(client);
    }else{
        Connect_Status = 0;
    }
}

void Timer_Init(){
    TimerHandle_t Weather_Refresh_Timer = xTimerCreate(
        "Refresh_Timer",
        pdMS_TO_TICKS(REFRESH_DELAY_MS),
        pdTRUE,
        NULL,
        Permission_Refresh_Data
    );
    
    if (Weather_Refresh_Timer == NULL) {
        printf("Timer create failed\n");
        return;
    }

    if (xTimerStart(Weather_Refresh_Timer, 0) != pdPASS) {
        printf("Timer start failed\n");
    }
}
