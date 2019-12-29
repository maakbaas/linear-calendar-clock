#include "ajax.h"
#include "esp_server.h"
#include "nvm.h"
#include "ArduinoJson.h"
#include <FS.h>

void ajax_begin();

void ajax_file_list(AsyncWebServerRequest *request);
void ajax_file_list_remove(AsyncWebServerRequest *request);
void ajax_remaining_size(AsyncWebServerRequest *request);

void ajax_esp_restart(AsyncWebServerRequest *request);
void ajax_wifi_update(AsyncWebServerRequest *request);
void ajax_wifi_details(AsyncWebServerRequest *request);

String src;

/*************************************
 * Bind JSON requests to server URLs *
 *************************************/

void ajax_begin()
{
    esp_server.on("/JSON/file_list", HTTP_GET, ajax_file_list);
    esp_server.on("/JSON/file_list_remove", HTTP_POST, ajax_file_list_remove);
    esp_server.on("/JSON/remaining_size", HTTP_GET, ajax_remaining_size);

    esp_server.on("/JSON/esp_restart", HTTP_POST, ajax_esp_restart);
    esp_server.on("/JSON/wifi_update", HTTP_POST, ajax_wifi_update);
    esp_server.on("/JSON/wifi_details", HTTP_GET, ajax_wifi_details);
}

/*************************************
 * Functions to handle JSON requests *
 *************************************/

/* send a file list to web interface */
void ajax_file_list(AsyncWebServerRequest *request)
{
    String JSON;
    StaticJsonDocument<1000> jsonBuffer;
    JsonArray files = jsonBuffer.createNestedArray("files");

    Dir dir = SPIFFS.openDir("");
    while (dir.next())
        files.add(dir.fileName().substring(1));
    
    serializeJson(jsonBuffer, JSON);

    request->send(200, "text/html", JSON);
}

/* remove a file */
void ajax_file_list_remove(AsyncWebServerRequest *request)
{
    SPIFFS.remove("/" + request->arg("filename"));
    request->send(200, "text/html", "");
}

/* send the disk usage information to web interface */
void ajax_remaining_size(AsyncWebServerRequest *request)
{
    String JSON;
    StaticJsonDocument<200> jsonBuffer;

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    jsonBuffer["used"] = String(fs_info.usedBytes);
    jsonBuffer["max"] = String(fs_info.totalBytes);
    serializeJson(jsonBuffer, JSON);
    
    request->send(200, "text/html", JSON);
}

/* restart the ESP */
void ajax_esp_restart(AsyncWebServerRequest *request)
{
    request->send(200, "text/html", "");
    delay(2000);
    ESP.reset();
}

/* update wifi details in the EEPROM */
void ajax_wifi_update(AsyncWebServerRequest *request)
{
    request->arg("ssid").toCharArray(ssid, sizeof(ssid) - 1);
    request->arg("pass").toCharArray(password, sizeof(password) - 1);
    write_EEPROM();
    request->send(200, "text/html", "");
}

/* send wifi details to web interface */
void ajax_wifi_details(AsyncWebServerRequest *request)
{
    String JSON;
    StaticJsonDocument<200> jsonBuffer;

    read_EEPROM();
    jsonBuffer["ssid"] = String(ssid);
    jsonBuffer["pass"] = String(password);   
    serializeJson(jsonBuffer, JSON);
    
    request->send(200, "text/html", JSON);
}
