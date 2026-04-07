idf_component_register(SRCS "HTTPS_Response.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client esp-tls json wifi_connect LCD_Interface) 