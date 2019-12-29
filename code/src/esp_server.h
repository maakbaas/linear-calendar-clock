#ifndef SERVER_H
#define SERVER_H

#include <ESPAsyncWebServer.h>

extern void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
extern void server_begin();

extern AsyncWebServer esp_server;
extern AsyncWebSocket ws;
extern int ws_client_counter;

#endif
