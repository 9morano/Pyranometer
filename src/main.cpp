#include <Arduino.h>
#include "project-config.h"
#include "pyrano_server.h"
#include "pyrano_wifi.h"
#include "ADXL350.h"

// Global variable to store the state of the server
uint8_t server_state = 1;
SemaphoreHandle_t server_mutex;


// Global time variable (8 char + /0)
char global_time[9];
SemaphoreHandle_t time_mutex;


// Global mesurement variables
float pitch = 0, roll = 0;	// Angle - inclination
static SemaphoreHandle_t inclination_mutex;


// Create objects
ADXL350 adxl = ADXL350();	// adxl to controll the accelerometer
pyranoWIFI wifi;			// wifi to controll radio and access point


// Task handles for task controll
TaskHandle_t task_handle_measure = NULL;



void toggleWiFi(uint8_t turnon);
void error(uint8_t reboot = 0);

void measureTask(void *param) {

	// Init accelerometer (defines in project-config.h)
	adxl.setup(ACC_OFFSET_X, ACC_OFFSET_Y, ACC_OFFSET_Z);
	delay(1);
	adxl.setRange(1);
	delay(1);
  	adxl.setDataRate(25);
	delay(1);
	adxl.begin();
	delay(1);

	while(1){

		// Get inclination
		xSemaphoreTake(inclination_mutex, portMAX_DELAY);
		adxl.getInclinationLPF(&pitch, &roll);
		//Serial.println(pitch);
		xSemaphoreGive(inclination_mutex);

		// Delay for 100ms
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}

// Update measurements
void updateTask(void *param){

	float _p = 0, _r = 0;

	while(1){

		// Update inclination measurements - store them in local variable because of mutex
		xSemaphoreTake(inclination_mutex, portMAX_DELAY);
		_p = pitch;
		_r = roll;
		xSemaphoreGive(inclination_mutex);

		SERVER_sendUpdatedMeasurements(1000, _p, _r, 0);

		// WebSockets on ESP32 can do max 15 emssages per second
		// https://github.com/me-no-dev/ESPAsyncWebServer/issues/504
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

void serverTask(void *param){

	uint8_t state = 1;

	
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
	// Turn on the server
	toggleWiFi(1);

	while(1){

		xSemaphoreTake(server_mutex, portMAX_DELAY);
		state = server_state;
		xSemaphoreGive(server_mutex);

		toggleWiFi(state);

		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void setup() {
	Serial.begin(SERIAL_BAUDRATE);
	delay(2000);

	

	inclination_mutex = xSemaphoreCreateMutex();
	time_mutex = xSemaphoreCreateMutex();
	server_mutex = xSemaphoreCreateMutex();

	xTaskCreate(
		serverTask,				// Function
		"Server task",			// Name (debugging purposes)
		10000,					// Stack size in bytes
		NULL,					// Parameters to pass
		11,						// Priority
		NULL					// Handler
	);

	xTaskCreate(
		measureTask,			// Function
		"Measurement task",		// Name (debugging purposes)
		2000,					// Stack size in bytes
		NULL,					// Parameters to pass
		10,						// Priority
		&task_handle_measure	// Handler
	);

	xTaskCreate(
		updateTask,				// Function
		"Update task",			// Name (debugging purposes)
		10000,					// Stack size in bytes
		NULL,					// Parameters to pass
		11,						// Priority
		NULL					// Handler
	);
	
}



void loop() {

	SERVER_cleanup();
	//Serial.println("Here");
	vTaskDelay(1000 / portTICK_PERIOD_MS);
}


// If error occurs, print something periodically to UART, so
// the UART LEDs will notify the user about it
void error(uint8_t reboot){
	// TODO: reboot after a while
    while (1) {
        if(millis() % 200 < 50){
            Serial.println("Error occurred..blinking LEDs!");
        }
    }
}

void toggleWiFi(uint8_t turnon) {

	if(turnon){
		if(!wifi.is_on){
			Serial.println("Turn on the wifi");
        	if(!wifi.APstart()){
            	Serial.println("Failed to start the AP");
            	error();
        	}
        	SERVER_start();
    	} 
		else {
			//Serial.println("WiFi is allready on!");
		}
	}
	else{
		if(wifi.is_on){
			Serial.println("Turn the wifi off");
			wifi.APstop();
        	SERVER_stop();
		}
		else{
			//Serial.println("WiFi is allready off!");
		}
	}

}