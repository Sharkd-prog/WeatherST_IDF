#pragma once

#define CITY_COUNT 2

#define CITY_COUNT_PERMITTED_REQUEST CITY_COUNT
#define WEATHER_DATA_COUNT 2
#define LENGTH_BUFFER 5
#define TEMP_DATA 0
#define HUMIDITY_DATA 1

#define INIT_CLIENT NULL

#define REFRESH_DELAY_MS 3600000

extern char weather_data[CITY_COUNT][WEATHER_DATA_COUNT][LENGTH_BUFFER];
extern int Connect_Status;
extern int Count_Request_Per_Hour[CITY_COUNT_PERMITTED_REQUEST];
extern const char* City[CITY_COUNT];

void Get_Weather_Data();
void Timer_Init();