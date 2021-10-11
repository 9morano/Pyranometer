/************************************************************************************
 * This is a costum library for Pyrano server configuration.
 * 
 * Before using it, upload the necessary static files to ESP32 FS (flash).
 * 
 * WebSocket messages are formed as JSON document with 2 pairs:
 *      - action ("a") defines the type of message
 *      - value ("v") holds the actual data
 * The message all together can be max 42 bytes long (for now).
 * Actions (enumerator in this file) must corelate with script.js.
 * 
 * Some variables are shared with main context - use semafor to access it.
 * 
 * Measurement task is form main context, but we can start / stop it from here, when
 * user sends coresponding commant - the measurement function is therefore extern.
 * 
 * @version 3
 * @author 9morano
 * @date 30.09.2021
 * @note Call SERVER_cleanup function every once in a while.
 ***********************************************************************************/


#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>


/*---------------------------------------------------------------------------------*/
// Static files and their location in FS
#define INDEX_FILE  "/index.html"
#define MEASUREMENTS_FILE  "/meritve.html"
#define CSS_FILE    "/src/style.css"
#define JS_FILE     "/src/script.js"
#define CSS_BS_FILE "/src/bootstrap.min.css"
#define JS_BS_FILE  "/src/bootstrap.bundle.min.js"
#define JS_JQ_FILE  "/src/jquery.min.js"


/*---------------------------------------------------------------------------------*/
// Actions of websocket messages
enum actions{
    REAL_TIME_DATA,
    SERVER_OFF,
    SERVER_TIME,
    DELETE_MEASUREMENT,
    START_MEASUREMENT,
    UPDATE_MEASUREMENT
};

/*---------------------------------------------------------------------------------*/
// Global varables shared with main code and server code
extern uint32_t global_time;
extern SemaphoreHandle_t mutex_time;

extern uint8_t server_state;
extern SemaphoreHandle_t mutex_server;

/*---------------------------------------------------------------------------------*/
// Variables for measurement task
extern TaskHandle_t task_handle_measure;
extern void measurementTask(void *param);

typedef struct m{
    char filename[13];
    uint16_t period;
    uint8_t restart;
} Measurement_t;


/*---------------------------------------------------------------------------------*/
/*----------------------------- SERVER FUNCTIONS ----------------------------------*/
/*---------------------------------------------------------------------------------*/

/**
 * Initializes the Async Web Server with HTTP request callbacks.
 * 
 * @return 1 if success, 0 otherwise
 * @todo fix return :)
 */
uint8_t SERVER_init(void);

/**
 * Starts the server interface
 * 
 * @todo fix return
 */
uint8_t SERVER_start(void);

/**
 * Stops the server interface
 * 
 * @todo fix return
 */
uint8_t SERVER_stop(void);

/**
 * This cleanup is a workaround, bc browser sometimes doesn't close the connection 
 * properly. If you want to save power, then calling as infrequently as once per second 
 * is sufficient.
 */
void SERVER_cleanup(void);

/**
 * It is used only the first time client opens html site to display variable strings.
 * @deprecated
 */
String SERVER_processor(const String& var);

/**
 * Handles WebSocket event.
 * 
 * @todo remove pritnf debug messages
 */
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

/**
 * Costum function to handle received data on WebSocket connection.
 * 
 * @note actions and values
 */
void SERVER_receiveWebSocketMessage(void *arg, uint8_t *data, size_t len);

/**
 * Costum function to send a formated message over WebSocket connection.
 */
uint8_t SERVER_sendWebSocketMessage(uint8_t action, const char *value);
