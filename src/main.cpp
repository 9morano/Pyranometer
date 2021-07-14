#include <Arduino.h>
#include "project-config.h"
#include "pyrano_server.h"
#include "pyrano_wifi.h"
#include "ADXL350.h"


ADXL350 adxl = ADXL350();

void measureTask(void *param) {

	// Init accelerometer (defines in project-config.h)
	adxl.setup(ACC_OFFSET_X, ACC_OFFSET_Y, ACC_OFFSET_Z);
	adxl.setRange(1);
  	adxl.setDataRate(100);
	adxl.begin();

	float pitch = 0, roll = 0;

	while(1){
		// Get inclination
		adxl.getInclination(&pitch, &roll);

		// Print it
		Serial.print("Pitch and roll: ");
		Serial.print(pitch);
		Serial.print(" ");
		Serial.println(roll);

		// Delay for 500ms
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}



// Create object wifi to control radio and access point
pyranoWIFI wifi;


void toggleWiFi(uint8_t turnon) {

	if(turnon){
		if(!wifi.is_on){
        	if(!wifi.APstart()){
            	Serial.println("Failed to start the AP");
            	error();
        	}
        	SERVER_start();
    	} 
		else {
			Serial.println("WiFi is allready on!");
		}
	}
	else{
		if(wifi.is_on){
			wifi.APstop();
        	SERVER_stop();
		}
		else{
			Serial.println("WiFi is allready off!");
		}
	}

}


// If error occurs, print something periodically to UART, so
// the UART LEDs will notify the user about it
void error(uint8_t reboot = 0){
	// TODO: reboot after a while
    while (1) {
        if(millis() % 200 < 50){
            Serial.println("Error occurred..blinking LEDs!");
        }
    }
}


void setup() {
	Serial.begin(SERIAL_BAUDRATE);
	delay(2000);

	// Init WiFi AP
    Serial.println("Configuring access point...");
    if(!wifi.init()){
        Serial.println("Failed to configure Access Point!");
        error();
    }

	// Display AP IP address
    Serial.print("Pyranometer IP address:");
    Serial.println(wifi.getIP());

	// Init server
    if(!SERVER_init()){
        Serial.println("An Error has occurred while init server (or mounting SPIFFS)");
        error();
    }

	// Turn the WiFi and the server ON
	toggleWiFi(1);
}



void loop() {

}