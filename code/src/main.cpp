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

void time();
void time_rainbow();
void calendar();
void daylight();
void goal(int percent);
void ticks_time(int hours, int minutes, int r, int g, int b, int width);

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
        ticks.setPixelColor(i, ticks.Color(0, 0, 0));        
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
    
    //main task, priority 1
    if (millis()-prev_time_task>2000 || start)
    {        
        prev_time_task = millis();
        start = false;

        // calendar();
        // time_rainbow();
        // goal(68);
        // daylight();
        time();
    }

}

//show the time as a green bar with 15min accuracy by using a yellow bar for quarters
void time()
{
    http.begin(client_ssl, "https://mb-clock.netlify.com/.netlify/functions/clock");

    String payload;
    if (http.GET() == HTTP_CODE_OK)
    {
        payload = http.getString();

        int hours = payload.substring(0, 2).toInt();
        int minutes = payload.substring(2, 4).toInt();

        float floatHours = (hours + (float)minutes / 60) * 2;

        ticks_time(hours, minutes, 22, 16, 10, hours*2);

        for (int i = 0; i < NUMBLOCKS; i++)
        {
            if (i + 1 <= floatHours)
                blocks.setPixelColor(i, blocks.Color(30, 30, 0));
            else if (i + 1 == ceil(floatHours) && minutes % 30 >= 15)
                blocks.setPixelColor(i, blocks.Color(60, 30, 0));
            else
                blocks.setPixelColor(i, blocks.Color(0, 0, 0));
        }

        ticks.show();
        blocks.show();
    }

    http.end();
}

//show the time as a rainbow bar
void time_rainbow()
{
    http.begin(client_ssl, "https://mb-clock.netlify.com/.netlify/functions/clock");

    String payload;
    if (http.GET() == HTTP_CODE_OK)
    {
        payload = http.getString();

        int hours = payload.substring(0, 2).toInt();
        int minutes = payload.substring(2, 4).toInt();

        float floatHours = (hours + (float)minutes / 60) * 2;

        ticks_time(hours, minutes, 22, 16, 10, hours * 2);

        for (int i = 0; i < NUMBLOCKS; i++)
        {
            if (i + 1 <= floatHours)
                blocks.setPixelColor(i, blocks.ColorHSV(i * 1365, 255, 30));
            else
                blocks.setPixelColor(i, blocks.Color(0, 0, 0));
        }

        ticks.show();
        blocks.show();
    }

    http.end();    
}

//show the calendar
void calendar()
{
    http.begin(client_ssl, "https://mb-clock.netlify.com/.netlify/functions/clock");

    String payload;
    if (http.GET() == HTTP_CODE_OK)
    {
        payload = "1450";//http.getString();

        int hours = payload.substring(0, 2).toInt();
        int minutes = payload.substring(2, 4).toInt();

        ticks_time(hours, minutes, 44, 32, 20, hours * 2);
        // ticks_time(hours, minutes, 30, 30, 0);
        ticks.show();
    }

    http.end();

    http.begin(client_ssl, "https://mb-clock.netlify.com/.netlify/functions/calendar");

    if (http.GET() == HTTP_CODE_OK)
    {
        payload = http.getString();

        for (int i=0; i<NUMBLOCKS; i++)
        {
            //color for calendar id 1 is different, can potentially assign a color for each calendar
            if (payload.substring(i, i+1).toInt()==1) 
                blocks.setPixelColor(i, blocks.Color(50, 5, 20));
            else if (payload.substring(i, i + 1).toInt()>0)
                blocks.setPixelColor(i, blocks.Color(5, 20, 20));
            else
                blocks.setPixelColor(i, blocks.Color(0, 0, 0));
        }

        blocks.show();
    }

    http.end();
}

//show the daylight
void daylight()
{
    String payload;
    http.begin(client_ssl, "https://mb-clock.netlify.com/.netlify/functions/daylight");

    if (http.GET() == HTTP_CODE_OK)
    {
        payload = http.getString();

        for (int i = 0; i < NUMBLOCKS; i++)
        {
            //color for twilight gradient
            if (payload.substring(i, i + 1).toInt() == 1)
                blocks.setPixelColor(i, blocks.Color(9,4, 11));
            else if (payload.substring(i, i + 1).toInt() == 2)
                blocks.setPixelColor(i, blocks.Color(13,8, 16));
            else if (payload.substring(i, i + 1).toInt() == 3)
                blocks.setPixelColor(i, blocks.Color(25,19,33));            
            else
                blocks.setPixelColor(i, blocks.Color(5, 0, 6));
        }

        blocks.show();
    }

    http.end();

    String payload2;
    http.begin(client_ssl, "https://mb-clock.netlify.com/.netlify/functions/clock");

    if (http.GET() == HTTP_CODE_OK)
    {
        payload2 = http.getString();

        int hours = payload2.substring(0, 2).toInt();
        int minutes = payload2.substring(2, 4).toInt();

        int bars = (hours + (float)minutes / 60) * 2;

        if (payload.substring(bars, bars + 1).toInt() == 1)
            ticks_time(hours, minutes, 65, 25, 15, 16);
        else if (payload.substring(bars, bars + 1).toInt() == 2)
            ticks_time(hours, minutes, 65, 25, 15, 16);
        else if (payload.substring(bars, bars + 1).toInt() == 3)
            ticks_time(hours, minutes, 60, 30, 10, 16);
        else
            ticks_time(hours, minutes, 44, 32, 20, 16);

        ticks.show();
    }

    http.end();
}

//goal tracker between 0 and 100%
void goal(int percent)
{
    percent/=2.0833;

    for (int i = 1; i < NUMTICKS; i += 2)
    {
        ticks.setPixelColor(i, ticks.Color(11, 8, 5));
    }

    for (int i = 0; i < NUMBLOCKS; i++)
    {
        if (i + 1 <= percent)
            blocks.setPixelColor(i, blocks.ColorHSV(i * 380, 255, 30));
        else
            blocks.setPixelColor(i, blocks.Color(0, 0, 0));
    }

    ticks.show();
    blocks.show();
}

//support function for ticks
void ticks_time(int hours, int minutes, int r, int g, int b, int width)
{
    float floatHours = (hours + (float)minutes / 60) * 2;

    for (int i = 0; i < NUMTICKS; i++)
    {
        if (i+1 <= floatHours)
        {
            int gradR = 11 + max(0, int((width - (floatHours - i)) * (r - 11) / (float)width));
            int gradG = 8 + max(0, int((width - (floatHours - i)) * (g - 8) / (float)width));
            int gradB = 5 + max(0, int((width - (floatHours - i)) * (b - 5) / (float)width));
            ticks.setPixelColor(i, ticks.Color(gradR, gradG, gradB));
        }
        else if (i <= floatHours)
            ticks.setPixelColor(i, ticks.Color(r, g, b));
        else
            ticks.setPixelColor(i, ticks.Color(11, 8, 5));
            //ticks.setPixelColor(i, ticks.Color(9, 7, 5));
    }
}