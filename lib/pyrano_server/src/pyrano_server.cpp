/* --------------------------------------------------------------------------------------------------
 * TODO
 * 
 * Ver.:  2
 * Auth.: Grega Morano
 * Date : 24.05.2021
 * About:
 * 
 * --------------------------------------------------------------------------------------------------*/

#include "Arduino.h"
#include "pyrano_server.h"


AsyncWebServer srvr(80);
AsyncWebSocket ws("/ws");

uint8_t SERVER_init(void)
{   

    uint8_t status;

    // Init websocket
    ws.onEvent(onEvent);
    srvr.addHandler(&ws);

    // Config the server
    // Route for root / web page
    srvr.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->redirect("/index.html");
        }
    );
    srvr.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, INDEX_FILE, String(), false, SERVER_processor);
        }
    );
    // When client sends request for graph site
    srvr.on("/graph.html", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, GRAPH_FILE, "text/html");
        }
    );
    // When client sends request for ccs file
    srvr.on("/src/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, CSS_FILE, "text/css");
        }
    );
    srvr.on("/src/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, CSS_BS_FILE, "text/css");
        }
    );

    // When client sends request for JS file
    srvr.on("/src/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, JS_FILE, "text/html");
        }
    );
    srvr.on("/src/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, JS_BS_FILE, "text/html");
        }
    ); 
    srvr.on("/src/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, JS_JQ_FILE, "text/html");
        }
    );   

    // When client wants to download a file of measurements
    srvr.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, "/measurements.txt", "text/plain", true); // true enables download option
        }
    // TODO: add a SD card
    );

    // When client request for unknown page, give him error 404
    srvr.onNotFound([](AsyncWebServerRequest *request)
        {
            request->send(404, "text/html", "<h1>404 Not found</h1>");
        }
    );

    // Init FFS
    status = SPIFFS.begin(true);

    // Check for neccesary files in FFS
    status &= SERVER_check4html();

    return status;
}

// TODO - do we need to check for existing files or not?
uint8_t SERVER_check4html(void)
{
    const char *filename;
    uint8_t html = 0, css = 0, js = 0, g = 0;

    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while(file){
        filename = file.name();

        if (strcmp(filename, INDEX_FILE) == 0){
            html = 1;
        }
        else if (strcmp(filename, CSS_FILE) == 0){
            css = 1;
        }
        else if (strcmp(filename, JS_FILE) == 0) {
            js = 1;
        }
        else if (strcmp(filename, GRAPH_FILE) == 0) {
            g = 1;
        }
        else{
            Serial.println("W: Found additional file:");
            Serial.println(filename);
            // TODO Delete it?
        }
        file = root.openNextFile();
    }

    if ((html && css) && (g && js)) {
        return 1;
    }
    else {
        Serial.printf("Found files: html %d, css %d, js %d, graf %d \n", html, css, js, g);
        return 0;
    }
}


uint8_t SERVER_start(void)
{
    srvr.begin();
    return 1;
}

uint8_t SERVER_stop(void)
{
    srvr.end();
    return 1;
}

void SERVER_cleanup(void)
{
    // This cleanup is a workaround, bc browser sometimes doesnt close the connection properly.
    // If you want to save power, then calling as infrequently as once per second is sufficient.
    ws.cleanupClients();
}

// It is used only the first time client opens html site to display variable strings
String SERVER_processor(const String& var)
{
    if(var == "STATE"){
        if (ledState){
            return "ON";
        }
        else{
            return "OFF";
        }
    }

    if(var == "A_VAL"){
        return String(pot_val);
    }

    return String();
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{

    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;

        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;

        case WS_EVT_DATA:
            Serial.printf("Received data from client %s: ", client->remoteIP().toString().c_str());
            SERVER_receiveWebSocketMessage(arg, data, len);
            break;

        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

// Handle socket message - whenever we receive somethin on it
void SERVER_receiveWebSocketMessage(void *arg, uint8_t *data, size_t len) 
{
    AwsFrameInfo *info = (AwsFrameInfo*)arg;

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

        // Same as while sending message
        const uint8_t size =  JSON_OBJECT_SIZE(2);
        StaticJsonDocument<size> json;
        DeserializationError err = deserializeJson(json, data);
        if (err) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.c_str());
        }

        const char *action = json["action"];

        if (strcmp(action, "ledstate") == 0){
            
            uint8_t value = json["value"];
            // Global variable
            ledState = value;

            // Notify all clients about the change
            SERVER_sendWebSocketMessage(UPDATE_LED, ledState);
            Serial.println("Received new led state");
            Serial.println(value);         
        }
        else if (strcmp(action, "time") == 0){
            const char *time = json["value"];
            Serial.print("Received clients time:");
            Serial.println(time);
        }

    }
}


// Action string length and value length must be max 20!
uint8_t SERVER_sendWebSocketMessage(uint8_t action, int value){
    
    // Define the number of elements used in JSON document
    const uint8_t size = JSON_OBJECT_SIZE(2);

    //StaticJsonDocument or DynamicJsonDocument --> stack or heap memory
    StaticJsonDocument<size> json;

    // Fill the json document
    switch (action){
        case UPDATE_LED:
            json["action"] = "ledstate";
            break;

        case UPDATE_AVAL:
            json["action"] = "aval";
            break;

        default:
            Serial.printf("ERROR: Unrecognised action %i \n", action);
            return 0;
    }

    json["value"] = value;

    // Define buffer for serialization of json data - char instead of Stirng, 
    // so we use stack instead of heap. Must be big enough (count chars)...
    // 13 for {",: ... +11 for keys ... +1 for string delimiter '/0'
    // + 10 for action and another 10 for value
    char data[45];

    size_t len = serializeJson(json, data);
    ws.textAll(data, len);

    return 1;
}