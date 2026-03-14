#include <stdio.h>
#include "nvs_flash.h"
#include "wifi_manage.h"
#include "esp_log.h"


void app_main(void)
{
    esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
    nvs_flash_init();

    WiFi_Connect_STA();
}
