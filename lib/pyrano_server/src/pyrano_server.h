/* --------------------------------------------------------------------------------------------------
 * TODO add #ifnotdefined
 * 
 * Ver.:  2
 * Auth.: Grega Morano
 * Date : 24.05.2021
 * About:
 * 
 * --------------------------------------------------------------------------------------------------*/


#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>


#define INDEX_FILE  "/index.html"
#define GRAPH_FILE  "/graph.html"
#define CSS_FILE    "/src/style.css"
#define JS_FILE     "/src/script.js"
#define CSS_BS_FILE "/src/bootstrap.min.css"
#define JS_BS_FILE  "/src/bootstrap.bundle.min.js"
#define JS_JQ_FILE  "/src/jquery.min.js"


// Definitions for possible message
#define UPDATE_LED  (1)
#define UPDATE_AVAL (2)

// Variables that can be modified through web interface
// Must be global in main INO file, linked to this file 
extern uint8_t ledState;
extern int pot_val;


// "Public" function for use in main INO file
uint8_t SERVER_init(void);
uint8_t SERVER_start(void);
uint8_t SERVER_stop(void);
void SERVER_cleanup(void);

uint8_t SERVER_sendWebSocketMessage(uint8_t action, int value);


// "Private" functions - used only in server.cpp
uint8_t SERVER_check4html(void);
String SERVER_processor(const String& var);

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void SERVER_receiveWebSocketMessage(void *arg, uint8_t *data, size_t len);
