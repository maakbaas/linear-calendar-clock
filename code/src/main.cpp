#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Adafruit_NeoPixel.h>

#include "ajax.h"
#include "esp_server.h"
#include "nvm.h"

#include <ESP8266HTTPClient.h>

WiFiClientSecure client_ssl;
HTTPClient http;

unsigned long prev_time_task;

#define NUMBLOCKS 48
#define NUMTICKS 49

void time(int hours, int minutes);
void time_rainbow(int hours, int minutes);
void daylight(int hours, int minutes);
void ticks_time(int hours, int minutes, int r, int g, int b);

Adafruit_NeoPixel blocks(NUMBLOCKS, 4, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ticks(NUMTICKS, 12, NEO_GRB + NEO_KHZ800);

bool start = true;

void setup()
{
    blocks.begin();
    ticks.begin();

    for (int i = 0; i < NUMBLOCKS; i++)
    {
        blocks.setPixelColor(i, blocks.Color(6,5,4));        
    }

    for (int i = 0; i < NUMTICKS; i++)
    {
        ticks.setPixelColor(i, ticks.Color(6, 5, 4));        
    }

    blocks.show();
    ticks.show();

    int start_time = millis();

    client_ssl.setInsecure();

    Serial.begin(115200);    
    
    pinMode(0, INPUT_PULLUP); 
    pinMode(5, INPUT_PULLUP);

    read_EEPROM();

    WiFi.softAPdisconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(200);

    WiFi.begin(ssid, password);

    int wifi_timeout = 0;

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        wifi_timeout++;
        if (wifi_timeout > 15)
        {
            WiFi.softAPdisconnect();
            WiFi.disconnect();
            WiFi.mode(WIFI_AP);
            delay(200);
            WiFi.softAP("ESP8266-Hotspot");
            break;
        }
    }

    SPIFFS.begin();
    ArduinoOTA.begin();

    server_begin();
    ajax_begin();

    //MDNS.begin("display");
    
    Serial.print("Setup duration: ");
    Serial.print(millis()-start_time);
    Serial.println("");

    prev_time_task = 0;
    
}

void loop()
{    
    ArduinoOTA.handle();
    
    //100ms task, priority 1
    if (millis()-prev_time_task>10000 || start)
    {        
        prev_time_task = millis();
        start = false;

        http.begin(client_ssl, "https://mb-clock.netlify.com/.netlify/functions/clock");

        String payload;
        if (http.GET() == HTTP_CODE_OK)
        {
            payload = http.getString();

            String hours = payload.substring(0, 2);
            String mins = payload.substring(2, 4);
            time_rainbow(hours.toInt(), mins.toInt());

            ticks.show();
            blocks.show();

        }

        http.end();

    }

}

void time(int hours, int minutes)
{
    float floatHours = (hours + (float)minutes / 60) * 2;

    ticks_time(hours, minutes, 22, 16, 10);

    for (int i = 0; i < NUMBLOCKS; i++)
    {
        if (i + 1 <= floatHours)
            blocks.setPixelColor(i, blocks.Color(30, 30, 0));
        else if (i + 1 == ceil(floatHours) && minutes % 30 >= 15)
            blocks.setPixelColor(i, blocks.Color(60, 30, 0));
        else 
            blocks.setPixelColor(i, blocks.Color(0, 0, 0));
    }
}

void time_rainbow(int hours, int minutes)
{
    float floatHours = (hours + (float)minutes / 60) * 2;

    ticks_time(hours, minutes, 22, 16, 10);

    for (int i = 0; i < NUMBLOCKS; i++)
    {
        if (i + 1 <= floatHours)
            blocks.setPixelColor(i, blocks.ColorHSV(i*1365,255,30));       
        else
            blocks.setPixelColor(i, blocks.Color(0, 0, 0));
    }
}

void daylight(int hours, int minutes)
{
    float floatHours = (hours + (float)minutes / 60) * 2;

    ticks_time(hours, minutes, 20, 0, 5);

    for (int i = 0; i < NUMBLOCKS; i++)
    {
        if (i + 1 <= floatHours)
            blocks.setPixelColor(i, blocks.ColorHSV(i * 1365, 255, 30));
        else
            blocks.setPixelColor(i, blocks.Color(0, 0, 0));
    }
}

void ticks_time(int hours, int minutes, int r, int g, int b)
{
    float floatHours = (hours + (float)minutes / 60) * 2;

    for (int i = 0; i < NUMTICKS; i++)
    {
        if (i+1 <= floatHours)
            ticks.setPixelColor(i, ticks.Color(22, 16, 10));
        else if (i <= floatHours)
            ticks.setPixelColor(i, ticks.Color(r, g, b));
        else
            ticks.setPixelColor(i, ticks.Color(22, 16, 10));
            //ticks.setPixelColor(i, ticks.Color(9, 7, 5));
    }
}