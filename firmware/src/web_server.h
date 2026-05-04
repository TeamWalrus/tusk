#pragma once

#include "card.h"
#include <Arduino.h>

namespace web_server {

// Initializes the routes. Call once during setup() before begin().
void setupRoutes();

// Starts the HTTP server.
void begin();

// Push a freshly captured card to all connected SSE clients.
void broadcastCard(const Card &card);

// Notify SSE clients that the historical card log was cleared.
void broadcastClear();

}  // namespace web_server
