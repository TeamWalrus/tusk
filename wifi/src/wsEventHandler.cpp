
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "wsEventHandler.h"

void wsEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {

        Serial.println("Websocket client connection received");
        client->text("Hello from ESP32 Server");
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.println("Client disconnected");
    }
}