#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Adafruit_NeoPixel.h>

#include "ajax.h"
#include "esp_server.h"
#include "nvm.h"

unsigned long prev_time_task;

#define NUMBLOCKS 49
#define NUMTICKS 48

Adafruit_NeoPixel blocks(NUMBLOCKS, D6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ticks(NUMTICKS, D2, NEO_GRB + NEO_KHZ800);

int k=0;

void setup()
{
    int start_time = millis();
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

    MDNS.begin("display");

    blocks.begin();
    blocks.show();
    // blocks.setBrightness(30);

    ticks.begin();
    ticks.show();
    // ticks.setBrightness(30);

    Serial.print("Setup duration: ");
    Serial.print(millis()-start_time);
    Serial.println("");

    prev_time_task = millis();
    
}

void loop()
{    
    ArduinoOTA.handle();
    
    //100ms task, priority 1
    if (millis()-prev_time_task>100)
    {
        prev_time_task = millis();
        
        //do task
        for (int i = 0; i < NUMBLOCKS; i++)
        {
            // blocks.setPixelColor(i, blocks.Color(200*(k>i), 0, 75));
            // if (i==21)
            // {
            //     blocks.setPixelColor(i, blocks.Color(20, 0, 0));
            // }
            // else
            // {
            //     blocks.setPixelColor(i, blocks.Color(20, 20, 10));
            // }

            blocks.setPixelColor(i, blocks.Color(random(0, 00), random(0, 40), random(0, 40)));
            blocks.show();
        }

        for (int i = 0; i < NUMTICKS; i++)
        {
            ticks.setPixelColor(i, ticks.Color(random(0, 40), random(0, 00), random(0, 00)));
            // ticks.setPixelColor(i, ticks.Color(30, 100 * (k > i), 0));
            ticks.show();
        }

        if (k<48)
            k++;
        else
            k=0;
    }

}
