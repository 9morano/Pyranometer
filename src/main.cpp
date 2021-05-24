#include <Arduino.h>
#include <pyrano_server.h>

// Create object wifi to control radio and access point
pWIFI wifi;


// Test variables
#define PIN_LED     (2)
#define PIN_BTN     (13)
#define PIN_POT     (33)
uint8_t ledState = 0;

uint8_t btnState = 0, lastBtnRead = 0;
unsigned long debounceTime = 0;

int pot_val = 0;
unsigned long pot_val_send_time = 0;
/* TODO:
 * ADC2 is not working when WiFi is in use...use ADC1 instead!
 * Pin is linked with GPIO name...so ADC1 CH0 == GPIO36 == 36
 */


// Function definitions
void error(void);
void toggleWiFi(void);

void setup(){
    
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BTN, INPUT_PULLUP);

    digitalWrite(PIN_LED, LOW);

    // Serial port for debugging purposes
    Serial.begin(9600);
    // Serial needs some time to initialize
    delay(100);


    // Init AP and start it
    Serial.println("Configuring access point...");
    if(!wifi.init()){
        Serial.println("Failed to configure Access Point!");
        error();
    }
    if(!wifi.APstart()){
        Serial.println("Failed to start the AP");
        error();
    }


    // Display AP IP address
    Serial.print("Pyranometer IP address:");
    Serial.println(wifi.getIP());


    // Init server and start it
    if(!SERVER_init()){
        Serial.println("An Error has occurred while init server (or mounting SPIFFS)");
        error();
    }
    SERVER_start();

}


void loop() {

    // Test for downloading a file
    /*File file = SPIFFS.open("/measurements.txt", FILE_WRITE);
    if (!file) Serial.println("Error opening file!");

    file.print("MEASUREMENTS");
    file.print("Measurement 1: ...");
    file.print("Measurement 2: ...");*/

    while(1) {
        SERVER_cleanup();

        // ----------------------------------------------------------------------------------------
        // Check button state
        uint8_t btnRead = digitalRead(PIN_BTN);

        if(btnRead != lastBtnRead){
            debounceTime = millis();
            lastBtnRead = btnRead;
        }

        if( (millis() - debounceTime) > 50){
            // If current read is different that current state
            if(btnRead != btnState){
                btnState = btnRead;

                // If button is pressed
                if (btnState == LOW){
                    Serial.println("Button pressed on NodeMCU");
                    

                    toggleWiFi();
                    // Change LED state
                    //ledState = !ledState;
                    // Notify all clients about the change
                    //SERVER_sendWebSocketMessage(UPDATE_LED, ledState);
                }
                // If button is released
                else{}
            }
        }

        // ----------------------------------------------------------------------------------------
        // Check analog value (test)

        // Every 100ms update pot value...MAX is 15 messages per second!
        if( (millis() - pot_val_send_time) > 1000){

            pot_val_send_time = millis();
            // Check analog value (test)
            pot_val = analogRead(PIN_POT);

            SERVER_sendWebSocketMessage(UPDATE_AVAL, pot_val);
            Serial.print("Pot value:");
            Serial.println(pot_val);
        }

        // ----------------------------------------------------------------------------------------
        // Set led accordingly to its state
        digitalWrite(PIN_LED, ledState);
    }
}

/* --------------------------------------------------------------------------------------------------- */

void error(void){
    while (1) {
        if(millis() % 200 < 50){
            digitalWrite(PIN_LED, HIGH);
        }
        else{
            digitalWrite(PIN_LED, LOW);
        }
    }
}


void toggleWiFi(void){

    if(wifi.is_on){
        wifi.APstop();
        SERVER_stop();
    }
    else{
        if(!wifi.APstart()){
            Serial.println("Failed to start the AP");
            error();
        }
        SERVER_start();
    }
}