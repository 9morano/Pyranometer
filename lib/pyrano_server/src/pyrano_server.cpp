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
    srvr.on("/meritve.html", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, MEASUREMENTS_FILE, "text/html");
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
            request->send(SPIFFS, JS_FILE, "text/javascript");
        }
    );
    srvr.on("/src/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, JS_BS_FILE, "text/javascript");
        }
    ); 
    srvr.on("/src/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(SPIFFS, JS_JQ_FILE, "text/javascript");
        }
    );   

    // When client wants to download a file of measurements
    // /download?file=<ime_datoteke.txt>
    srvr.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
        { 
            String input_filename;
            Serial.print("User requested for measurements:");
            if(request->hasParam("file")){
                input_filename = request->getParam("file")->value();
                input_filename = "/measure/" + input_filename;
                Serial.println(input_filename);
                request->send(SPIFFS, input_filename, "text/plain", true); // true enables download option
            }
        }
    );

    // When client request for unknown page, give him error 404
    srvr.onNotFound([](AsyncWebServerRequest *request)
        {
            request->send(404, "text/html", "<h1>404 Not found</h1>");
        }
    );

    status = 1;

    return status;
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
// TODO: remove this...return raw HTML without this processor - no need for it
String SERVER_processor(const String& var)
{
    /*if(var == "STATE"){
        if (ledState){
            return "ON";
        }
        else{
            return "OFF";
        }
    }

    if(var == "A_VAL"){
        return String(pot_val);
    }*/

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

        //const char *action = json["a"];

        uint8_t action = json["a"];

        switch(action){

            case SERVER_MANAGEMENT:
            {
                uint8_t value = json["v"];

                if(value == 0){
                    Serial.println("Turn off the server.");
                    xSemaphoreTake(mutex_server, portMAX_DELAY);
                    server_state = 0;
                    xSemaphoreGive(mutex_server);
                }
                else if(value == 1){
                    // Get measurement filenames and send them to the client
                    File measurements = SPIFFS.open("/measure");

                    while(true){
                        File file = measurements.openNextFile();
                        
                        if(!file){
                            break;
                        }

                        // You could also get the size of the file...
                        const char *str = file.name();

                        // With +9 remove "/measure/" from the filename
                        SERVER_sendWebSocketMessage(UPDATE_FILENAME, str+9);
                        file.close();
                    }
                }
            }
            break;

            case NEW_TIME:
            {   
                uint8_t h = 0, m = 0, s = 0;
                char *tmp;
                char tmp_char[10];
                uint32_t new_time = 0;

                // TODO: delete this - debug
                const char *t = json["v"];
                Serial.print("Received clients time:");
                Serial.println(t);

                strcpy(tmp_char, t);

                // Convert time from char array to ints
                tmp = strtok(tmp_char, ":");
                if(tmp){
                    h = atoi(tmp);
                }
                tmp = strtok(NULL, ":");
                if(tmp){
                    m = atoi(tmp);
                }
                tmp = strtok(NULL, ":");
                if(tmp){
                    s = atoi(tmp);
                }

                // Update global time variable
                new_time |= (s);
                new_time |= (m) << 8;
                new_time |= (h) << 16;
                new_time |= (0x01) << 24;   // Update indicator 

                xSemaphoreTake(mutex_time, portMAX_DELAY);
                global_time = new_time;
                xSemaphoreGive(mutex_time);          
            }
            break;

            case NEW_FILE:
                // Inform main context about new filename
                
                // Confirm to the client
                //SERVER_sendWebSocketMessage(UPDATE_FILENAME, str+9);

            break;

            case DELETE_FILE:
            {
                const char *path = json["v"];
                char p[30] = "/measure/";
                strcat(p, path);
                Serial.printf("User deleted file %s \n", p);
                SPIFFS.remove(p);
            }
            break;

            default:
                // TODO brisi, ne rabim defaulta ko bo apk koncana
                Serial.print("Received unknown action from the client:");
                Serial.print(action);
                break;
        }
    }
}


// Value length must be max 18 chars
uint8_t SERVER_sendWebSocketMessage(uint8_t action, const char *value){
    
    // Define the number of elements used in JSON document
    const uint8_t size = JSON_OBJECT_SIZE(2);

    //StaticJsonDocument or DynamicJsonDocument --> stack or heap memory
    StaticJsonDocument<size> json;

    // Fill the json document
    switch (action){
        case UPDATE_MEASUREMENT:
        case UPDATE_FILENAME:
            json["a"] = action;
            json["v"] = value;
            break;

        default:
            // TODO brisi
            Serial.printf("ERROR: Unrecognised action %i \n", action);
            return 0;
    }

    // Serialize json document and send it via WS
    char data[40];
    size_t len = serializeJson(json, data);
    ws.textAll(data, len);

    return 1;
}




uint8_t SERVER_sendUpdatedMeasurements(float power, float pitch, float roll, uint8_t temp){

    /* Possible measurements and their max values:
     * --------------
     * power: 1000.0
     * pitch: -90.0
     * roll : -90.0
     * temp : 70
     * --------------
     * together it takes 18 chars for measurements + 3 for delimiters (|)
     * 
     * example: {"a":1,"v":"0000|-24.0|-24.0|66.0"}
     * there is 37 characters +1 for /0 delimiter - taking 40 just in case
     */

    char str[22];   // 21 char + /0

    // Store the values into one string
    sprintf(str, "%4.1f|%3.1f|%3.1f|%d", power, pitch, roll, temp);

    SERVER_sendWebSocketMessage(UPDATE_MEASUREMENT, str);
    return 1;
}


