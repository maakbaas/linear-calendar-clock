#include "esp_server.h"
#include <FS.h>

File fsUploadFile;
AsyncWebServer esp_server(80);
int ws_client_counter = 0;

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void server_begin();

void server_begin()
{    
    esp_server.begin();

    /* handle for default files */
    esp_server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    esp_server.onNotFound([](AsyncWebServerRequest *request) {

        request->send(404);

    });

    /* handle for uploads */
    esp_server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) { request->send(200); },
              handleFileUpload);
}
   
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!index)
    {
        if (!filename.startsWith("/"))
            filename = "/" + filename;

        fsUploadFile = SPIFFS.open(filename, "w");
    }

    for (size_t i = 0; i < len; i++)
    {
        fsUploadFile.write(data[i]);
    }

    if (final)
    {
        fsUploadFile.close();

        AsyncWebServerResponse *response = request->beginResponse(303);
        response->addHeader("Location", "/");
        request->send(response);
    }
}