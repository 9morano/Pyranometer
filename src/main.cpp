
// Board specific configuration are located in /include/pyrano-config.h



/* SHORT DESCRIPTION:
------------------------------------------------------------------------------------------------------
AT STARTUP:
* init ACC - start inclination task. Maybe use it instead of interupts to detect activity :)
* init ADC - not necessary, but preferable
* init FS - only init, do not start doing msmnts
* init SERVER - start wifi AP and server and wait for client
* start updateServer task - to show the user msmnts

WHEN USER SENDS START CMD: (new measurement)
* obtain filename, period and clients time, then start the measurement task. Use the period to call the 
function repeatedly and update the timestamp accordingly

WHEN USER SENDS DOWNLOAD CMD:
* send the CSV file to the user and optionally the stop measurement task

WHEN ACTIVITY DETECTED:
* turn the server on, start the updateServer task

AFTER 3MIN OF INACTIVITY:
* turn the server off, stop the update task, continue with msmnt task if there is any allready running

------------------------------------------------------------------------------------------------------

FROM MAIN LOOP TO SERVERSIDE:
* Warning that measurement is full
* warning that measurements cant be saved (could not create file?)

* Update measurements periodically
* send csv file

FROM USER TO SERVERSIDE:
* CMD: start measurement with arg: filename, period and clients time
* CMD: download the file
* CMD: delete file
* CMD: turn off the server

------------------------------------------------------------------------------------------------------
TODO:
* If you use Serial print, increase stack size at task startup

* add calibration at each startup
* add software reset option with esp_restart() or maybe ESP.restart()

* When activity detected, server goes on ... but what if the user doesnt connect to it, how does it turn off -.-

* check stack size with Serial.println(uxTaskGetStackHighWaterMark(NULL)); --> tells how much stack is left there

*/
#include <Arduino.h>
#include "pyrano-config.h"
#include "pyrano_server.h"
#include "pyrano_wifi.h"
#include "pyrano_fs.h"
#include "ADXL350.h"
#include "MCP3421.h"

// Global variable to store the state of the server
uint8_t server_state = 1;
SemaphoreHandle_t mutex_server;


// Global time variable 0x00hhmmss
uint32_t global_time = 0;
SemaphoreHandle_t mutex_time;


// Global mesurement variables
float pitch_g = 0, roll_g = 0, powa_g = 0;
static SemaphoreHandle_t mutex_inclination;


// Create objects
ADXL350 adxl = ADXL350();	// adxl to controll the accelerometer
MCP3421 adc = MCP3421();
pyranoWIFI wifi;			// wifi to controll radio and access point
FileSystem m_file;


// Task handles for task controll
TaskHandle_t task_handle_measure = NULL;
TaskHandle_t task_handle_update = NULL;
TaskHandle_t task_handle_time = NULL;


void toggleWiFi(uint8_t turnon);
void error(uint8_t reboot = 0);


/* Inclination task must occur frequently to obtain true value of the angle ...
 * 25 samples per second seems reasonable - therefore repeat this process 
 * 20 times per second. Values are stored globlay - access it with mutex_inclination
 */
void inclinationTask(void *param) {

	float powa = 0, pitch = 0, roll = 0;

	adc.startOneshot();

	while(1){

		// Optain measurements
		adxl.getInclination(&pitch, &roll);
		adc.getPowa(&powa);

		// Apply LPF - low pass filter
		xSemaphoreTake(mutex_inclination, portMAX_DELAY);
		powa_g = powa_g * 0.9 + powa * 0.1;
		roll_g = roll_g * 0.9 + roll * 0.1;
		pitch_g = pitch_g * 0.9 + pitch * 0.1;
		xSemaphoreGive(mutex_inclination);
		//Serial.println(pitch_g);

		// Check for activity interupt
		if(digitalRead(14)){
			if(adxl.getActivity()){
				//Serial.println("Activity detected!");
				xSemaphoreTake(mutex_server, portMAX_DELAY);
                server_state = 1;
                xSemaphoreGive(mutex_server);
				toggleWiFi(1);
			}
		}
		adc.startOneshot();

		// Delay for 100ms
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}


/* Task to send updates to the server - every 200ms
 * Possible measurements and their max values:
	* --------------
	* power: 1000.0
	* pitch: -90.0
	* roll : -90.0
	* temp : 70
	* --------------
	* together it takes 18 chars for measurements + 3 for delimiters (|)
 */
void serverUpdateTask(void *param){

	char str[22];   // 21 char + /0
	float w = 0, p = 0, r = 0;

	while(1){

		// Obtain the temperature 
		xSemaphoreTake(mutex_inclination, portMAX_DELAY);
		p = pitch_g;
		r = roll_g;
		w = powa_g;
		xSemaphoreGive(mutex_inclination);

		// Store the values into one string
		sprintf(str, "%4.1f|%3.1f|%3.1f|%d", w, p, r, 0);

		SERVER_sendWebSocketMessage(REAL_TIME_DATA, str);

		// WebSockets on ESP32 can do max 15 emssages per second
		// https://github.com/me-no-dev/ESPAsyncWebServer/issues/504
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

void serverPowerTask(void *param){

	uint8_t state = 1;

	// Init WiFi AP
    //Serial.println("Configuring access point...");
    if(!wifi.init()){
        Serial.println("Failed to configure Access Point!");
        error();
    }

	// Display AP IP address
    //Serial.print("Pyranometer IP address:");
    //Serial.println(wifi.getIP());

	// Init server
    if(!SERVER_init()){
        Serial.println("An Error has occurred while init server (or mounting SPIFFS)");
        error();
    }
	// Turn on the server
	toggleWiFi(1);

	while(1){

		xSemaphoreTake(mutex_server, portMAX_DELAY);
		state = server_state;
		xSemaphoreGive(mutex_server);

		// Wait for some time, before you confirm server OFF state.
		// When user changes sites (index.html -> meritve.html)
		// Sockets loose connection, therefore server gets state 0
		// For a short period of time (apx. 3-7 sec)
		if(state == 0){
			vTaskDelay(10000 / portTICK_PERIOD_MS);

			xSemaphoreTake(mutex_server, portMAX_DELAY);
			state = server_state;
			xSemaphoreGive(mutex_server);

			if(state == 0){
				toggleWiFi(state);
			}
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void toggleWiFi(uint8_t turnon) {
	if(turnon){
		if(!wifi.is_on){
			Serial.println("Wifi On");
        	if(!wifi.APstart()){
            	Serial.println("Failed to start the AP");
            	error();
        	}
        	SERVER_start();
			
			xTaskCreate(
				serverUpdateTask,		// Function
				"Update task",			// Name (debugging purposes)
				4096,					// Stack size in bytes
				NULL,					// Parameters to pass
				8,						// Priority
				&task_handle_update		// Handler
			);
    	} 
	}
	else{
		if(wifi.is_on){
			Serial.println("WiFi off");
			wifi.APstop();
        	SERVER_stop();
			vTaskDelete(task_handle_update);
		}
	}
}

/* Task to update global time counter
 * When client connects to the server, the tim is updated - indicated
 * with bit 24 setted to 1.
 * This kind of time counter is quite unaccurate..oh well
*/ 
void timeCounterTask(void *param){

	uint8_t h = 0, m = 0, s = 0;
	uint32_t time = 0;

	while(1){
		xSemaphoreTake(mutex_time, portMAX_DELAY);

		// If time was updated - bit 24 set to 1
		if((global_time >> 24) == 1){
			s = (uint8_t)(global_time & 0xff);
			m = (uint8_t)((global_time >> 8 ) & 0xff);
			h = (uint8_t)((global_time >> 16) & 0xff);
		}

		s++;
		if(s == 60){
			s = 0;
			m++;
			if(m == 60){
				m = 0;
				h++;
				if(h == 24){
					h = 0;
				}
			}
		}
		time = 0;
		time |= (s);
		time |= (m) << 8;
		time |= (h) << 16;

		global_time = time;
		xSemaphoreGive(mutex_time);
		//Serial.printf("Time = %d:%d:%d \n", h, m, s);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

/* Task intendend for obtaining the measurements of ADC and TEMP and 
 * storing them into a file. Measurement period must be given as 
 * param to the function when started. Give it in seconds!
 * Use inclination mutex to prevent simultaneous access to resources
 * from other tasks...inclination may suffer, but what can you do
*/
void measurementTask(void *param){

	float powa = 0, pitch = 0, roll = 0;
	uint8_t temp = 0;
	uint32_t time = 0;
	uint8_t s = 0, m = 0, h = 0;

	char filename[10];
	uint16_t period = 0;
	Measurement_t *msmnt = (Measurement_t *)param;
	strncpy(filename, msmnt->filename, 12);
	period = msmnt->period;

	m_file.createFile(filename);

	while(1){

		// Get the inclination and TODO: temperature
		xSemaphoreTake(mutex_inclination, portMAX_DELAY);
		pitch = pitch_g;
		roll = roll_g;
		powa = powa_g;
		xSemaphoreGive(mutex_inclination);

		// Get timestamp
		xSemaphoreTake(mutex_time, portMAX_DELAY);
		time = global_time;
		xSemaphoreGive(mutex_time);

		s = (uint8_t)(time & 0xff);
		m = (uint8_t)((time >> 8 ) & 0xff);
		h = (uint8_t)((time >> 16) & 0xff);

		if(m_file.storeMeasurementWTimstamp(&powa, &pitch, &roll, &temp, &h, &m, &s) != 1){
			// TODO: if we get error, memory is most likely full
			// Stop with the measurement task and inform user if possible
			Serial.println("WARNING: Failed to store msmnt!");
			vTaskDelete(NULL);
		}

		vTaskDelay(period * 1000 / portTICK_PERIOD_MS);
	}
}

void setup() {
	Serial.begin(SERIAL_BAUDRATE);
	delay(2000);

	m_file.setup();
	m_file.printInfo();

	// Init accelerometer (defines in project-config.h)
	adxl.setup(ACC_OFFSET_X, ACC_OFFSET_Y, ACC_OFFSET_Z);
	adxl.setRange(1);
  	adxl.setDataRate(25);
	pinMode(14, INPUT_PULLUP);
	adxl.initActivityInt();
	// Confirm correct setup.
	//adxl.printAllRegister();
	adxl.begin();

	// Init analog to digital converter
	adc.setup();

	mutex_inclination = xSemaphoreCreateMutex();
	mutex_time = xSemaphoreCreateMutex();
	mutex_server = xSemaphoreCreateMutex();

	xTaskCreate(
		serverPowerTask,		// Function
		"Server task",			// Name (debugging purposes)
		4096,					// Stack size in bytes
		NULL,					// Parameters to pass
		11,						// Priority
		NULL					// Handler
	);

	xTaskCreatePinnedToCore(
		inclinationTask,		// Function
		"Inclination task",		// Name (debugging purposes)
		2000,					// Stack size in bytes (1024 should do)
		NULL,					// Parameters to pass
		10,						// Priority
		NULL,	// Handler
		1
	);

	xTaskCreatePinnedToCore(
		timeCounterTask,
		"Time counter",
		1000,
		NULL,
		20,
		NULL,
		1
	);

	// Start default measurement file with period of 10 minutes
	Measurement_t measurement;
	strncpy(measurement.filename, "31_07_2021", 12);
	measurement.period = 10;

	xTaskCreatePinnedToCore(
		measurementTask,		// Function
		"Measurement task",			// Name (debugging purposes)
		16000,					// Stack size in bytes
		(void *) &measurement,					// Parameters to pass
		9,						// Priority
		&task_handle_measure,					// Handler
		1
	);
}


/**
 * Main (IDLE) loop - taking care of server workaround.
 */
void loop() {
	SERVER_cleanup();
	vTaskDelay(1000 / portTICK_PERIOD_MS);
}


/**
 * Development purpose ..
 */
void error(uint8_t reboot){
    configASSERT(0);
}

