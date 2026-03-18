#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "wifi_manage.h"

static const char* TAG_Wifi= "WiFi";
static const char* TAG_IP = "IP";
static uint8_t Retry_Connect = 0;
static EventGroupHandle_t WiFi_Event_Group;

wifi_config_t WiFi_cfg = {
        .sta = {
            .ssid ="TP-Link_FD24",
            .password = "01187441"
        }
    };

static void handle_wifi_event(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){

    if(event_id == WIFI_EVENT_STA_START){
        ESP_LOGI(TAG_Wifi,"WiFi Start");
        esp_wifi_connect();
    }
    if(event_id == WIFI_EVENT_STA_STOP){
        ESP_LOGI(TAG_Wifi,"WiFi Stop");
    }
    if(event_id == WIFI_EVENT_STA_CONNECTED){
        ESP_LOGI(TAG_Wifi,"WiFi Connect to network %s",WiFi_cfg.sta.ssid);
    }
    if(event_id == WIFI_EVENT_STA_DISCONNECTED){
        if(Retry_Connect < 1){
            ESP_LOGI(TAG_Wifi,"WiFi Disconnected from network %s",WiFi_cfg.sta.ssid);
        }
        xEventGroupSetBits(WiFi_Event_Group, WIFI_DISCONNECT_BIT);
    }

}

static void handle_ip_event(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    if(event_id == IP_EVENT_STA_GOT_IP){
        ESP_LOGI(TAG_IP,"IP address ready");
        xEventGroupSetBits(WiFi_Event_Group, WIFI_CONNECT_BIT);
    }
    if(event_id == IP_EVENT_STA_LOST_IP){
        ESP_LOGI(TAG_IP,"IP address lost");
        xEventGroupClearBits(WiFi_Event_Group, WIFI_CONNECT_BIT);
    }
    
}

void WiFi_Reconnect_Task(void* const pvParameter){
    Retry_Connect = 0;

    while(1){
        EventBits_t bit = xEventGroupWaitBits(
            WiFi_Event_Group,
            WIFI_DISCONNECT_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );
        if(bit & WIFI_DISCONNECT_BIT){
            if(Retry_Connect < RETRY_CONNECT_LIMIT){
                Retry_Connect++;
                ESP_LOGI(TAG_Wifi,"Reconnecting to the network %s (attempt %d)", WiFi_cfg.sta.ssid, Retry_Connect );
                esp_wifi_connect();
                vTaskDelay(pdMS_TO_TICKS(TRY_CONNECT_DELAY));
            }else{
                ESP_LOGW(TAG_Wifi,"Max attempts. Connection attempts are available avery %d seconds",LONG_TRY_CONNECT_DELAY/1000);
                vTaskDelay(pdMS_TO_TICKS(LONG_TRY_CONNECT_DELAY));
                Retry_Connect = 0;
                esp_wifi_connect();
            }
        }
    }
}

bool Wait_connect_to_wifi(){
    EventBits_t bit = xEventGroupWaitBits(
            WiFi_Event_Group,
            WIFI_CONNECT_BIT ,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );
        if(bit & WiFi_CONNECTED){
            return WiFi_CONNECTED;
        }
        return NULL;
}

void WiFi_Connect_STA(){
    esp_netif_init();

    esp_event_loop_create_default();

    esp_netif_create_default_wifi_sta();

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, handle_wifi_event,NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, handle_ip_event, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, handle_ip_event, NULL);

    WiFi_Event_Group = xEventGroupCreate();
    TaskHandle_t Task_handle;
    xTaskCreate(WiFi_Reconnect_Task, "WiFi_Reconnect_Task", 4096, NULL, 1, &Task_handle);

    wifi_init_config_t WiFi_init = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&WiFi_init);

    esp_wifi_set_config(WIFI_IF_STA, &WiFi_cfg);

    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_wifi_start();
}
