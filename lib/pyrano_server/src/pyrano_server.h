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
enum actions{UPDATE_MEASUREMENT, UPDATE_GRAPH, SERVER_MANAGEMENT, UPDATE_TIME};

// Global varables shared with main code and server code
extern char global_time[9];
extern SemaphoreHandle_t time_mutex;

extern uint8_t server_state;
extern SemaphoreHandle_t server_mutex;



// Function definitions
uint8_t SERVER_init(void);
uint8_t SERVER_start(void);
uint8_t SERVER_stop(void);
void SERVER_cleanup(void);

uint8_t SERVER_sendWebSocketMessage(uint8_t action, const char *value);
uint8_t SERVER_sendUpdatedMeasurements(uint16_t power, float pitch, float roll, uint8_t temp);

String SERVER_processor(const String& var);

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void SERVER_receiveWebSocketMessage(void *arg, uint8_t *data, size_t len);
