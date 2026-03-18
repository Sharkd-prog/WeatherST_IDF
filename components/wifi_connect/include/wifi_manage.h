
#define RETRY_CONNECT_LIMIT 5
#define TRY_CONNECT_DELAY 3000 
#define LONG_TRY_CONNECT_DELAY 10000

#define WIFI_DISCONNECT_BIT BIT0
#define WIFI_CONNECT_BIT BIT1

#define WiFi_CONNECTED true
#define WiFi_DISCONNECTED false

bool Wait_connect_to_wifi();
void WiFi_Connect_STA();