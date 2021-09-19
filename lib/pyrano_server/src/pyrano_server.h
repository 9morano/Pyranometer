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
#define MEASUREMENTS_FILE  "/meritve.html"
#define CSS_FILE    "/src/style.css"
#define JS_FILE     "/src/script.js"
#define CSS_BS_FILE "/src/bootstrap.min.css"
#define JS_BS_FILE  "/src/bootstrap.bundle.min.js"
#define JS_JQ_FILE  "/src/jquery.min.js"


// Definitions for possible message
enum actions{
    REAL_TIME_DATA,
    SERVER_OFF,
    SERVER_TIME,
    DELETE_MEASUREMENT,
    START_MEASUREMENT,
    UPDATE_MEASUREMENT
};


// Global varables shared with main code and server code
extern uint32_t global_time;
extern SemaphoreHandle_t mutex_time;

extern uint8_t server_state;
extern SemaphoreHandle_t mutex_server;

// Exteren variables for measurement task
extern TaskHandle_t task_handle_measure;
extern void measurementTask(void *param);

typedef struct m{
    char filename[13];
    uint16_t period;
    uint8_t restart;
} Measurement_t;



// Function definitions
uint8_t SERVER_init(void);
uint8_t SERVER_start(void);
uint8_t SERVER_stop(void);
void SERVER_cleanup(void);

String SERVER_processor(const String& var);

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void SERVER_receiveWebSocketMessage(void *arg, uint8_t *data, size_t len);
uint8_t SERVER_sendWebSocketMessage(uint8_t action, const char *value);
