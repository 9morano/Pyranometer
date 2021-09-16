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
* TODO: check if the msmnt is allready running - stop it in that case
* TODO: confirm to the user

WHEN USER SENDS DOWNLOAD CMD:
* send the CSV file to the user and optionally the stop measurement task

WHEN ACTIVITY DETECTED:
* turn the server on, start the updateServer task

AFTER 3MIN OF INACTIVITY:
* turn the server off, stop the update task, continue with msmnt task if there is any allready running

------------------------------------------------------------------------------------------------------

FROM MAIN LOOP TO SERVERSIDE:
* Warning that measurement is full
* warning that measurements cant saved (could not create file?)

* Update measurements periodically
* send csv file

FROM USER TO SERVERSIDE:
* CMD: start measurement with arg: filename, period and clients time
* CMD: download the file
* CMD: delete file
* CMD: turn off the server

------------------------------------------------------------------------------------------------------
TODO:
* add timestamp to the measurements file
* some errors occurs at startup
* add calibration at each startup

*/
#include <Arduino.h>
#include "project-config.h"
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
float pitch_g = 0, roll_g = 0;	// Angle - inclination
static SemaphoreHandle_t mutex_inclination;


// Create objects
ADXL350 adxl = ADXL350();	// adxl to controll the accelerometer
MCP3421 adc = MCP3421();
pyranoWIFI wifi;			// wifi to controll radio and access point
FileSystem m_file;


// Task handles for task controll
TaskHandle_t task_handle_measure = NULL;
TaskHandle_t task_handle_time = NULL;


void toggleWiFi(uint8_t turnon);
void error(uint8_t reboot = 0);


/* Inclination task must occur frequently to obtain true value of the angle ...
 * 25 samples per second seems reasonable - therefore repeat this process 
 * 20 times per second. Values are stored globlay - access it with mutex_inclination
 */
void inclinationTask(void *param) {

	// Init accelerometer (defines in project-config.h)
	adxl.setup(ACC_OFFSET_X, ACC_OFFSET_Y, ACC_OFFSET_Z);
	adxl.setRange(1);
  	adxl.setDataRate(25);

	//adxl.printAllRegister();
	// TODO: Confirm correct setup.
	adxl.begin();


	while(1){
		// Get inclination
		xSemaphoreTake(mutex_inclination, portMAX_DELAY);
		adxl.getInclinationLPF(&pitch_g, &roll_g);
		//Serial.println(pitch_g);
		xSemaphoreGive(mutex_inclination);

		// Delay for 50ms
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}


/* Task to send updates to the server - every 200ms
 * Use inclination mutex to prevent simultaneous access to resources
 * from other tasks...inclination may suffer, but I'm not payed for this
 */
void serverUpdateTask(void *param){

	float powa = 0, p = 0, r = 0;

	while(1){

		xSemaphoreTake(mutex_inclination, portMAX_DELAY);
		adc.startOneshot();
		p = pitch_g;
		r = roll_g;
		adc.getPowa(&powa);
		xSemaphoreGive(mutex_inclination);

		//Serial.println(p);

		SERVER_sendUpdatedMeasurements(powa, p, r, 0);

		// WebSockets on ESP32 can do max 15 emssages per second
		// https://github.com/me-no-dev/ESPAsyncWebServer/issues/504
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}



/* Task intendend for obtaining the measurements of ADC and TEMP and 
 * storing them into a file. Measurement period must be given as 
 * param to the function when started. Give it in seconds!
 * Use inclination mutex to prevent simultaneous access to resources
 * from other tasks...inclination may suffer, but what can you do
*/
void measurementTask(uint16_t period){

	float powa = 0, pitch = 0, roll = 0;
	uint8_t temp = 0;

	while(1){
		xSemaphoreTake(mutex_inclination, portMAX_DELAY);
		adc.startOneshot();
		// Obtain the temperature 
		pitch = pitch_g;
		roll = roll_g;
		adc.getPowa(&powa);
		xSemaphoreGive(mutex_inclination);

		if(m_file.storeMeasurement(powa, pitch, roll, temp) != 1){
			// TODO: if we get error, memory is most likely full
			// Stop with the measurement task and inform user if possible
			Serial.println("WARNING: Failed to store msmnt!");
			vTaskDelete(NULL);
		}

		vTaskDelay(period * 1000 / portTICK_PERIOD_MS);
	}
}

void serverPowerTask(void *param){

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

		xSemaphoreTake(mutex_server, portMAX_DELAY);
		state = server_state;
		xSemaphoreGive(mutex_server);

		toggleWiFi(state);

		vTaskDelay(100 / portTICK_PERIOD_MS);
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
		// 1 00010000 00011111 00000011
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

void setup() {
	Serial.begin(SERIAL_BAUDRATE);
	delay(2000);

	m_file.setup();

	adc.setup();

	// TODO:
	// Changes stored filename and creates a file
	//m_file.changeCurrentFilename("01_08_2022");
	//m_file.printMeasurementsFiles();


	mutex_inclination = xSemaphoreCreateMutex();
	mutex_time = xSemaphoreCreateMutex();
	mutex_server = xSemaphoreCreateMutex();

	xTaskCreate(
		serverPowerTask,		// Function
		"Server task",			// Name (debugging purposes)
		10000,					// Stack size in bytes
		NULL,					// Parameters to pass
		11,						// Priority
		NULL					// Handler
	);

	xTaskCreate(
		inclinationTask,		// Function
		"Inclination task",		// Name (debugging purposes)
		2000,					// Stack size in bytes
		NULL,					// Parameters to pass
		10,						// Priority
		&task_handle_measure	// Handler
	);

	xTaskCreate(
		serverUpdateTask,		// Function
		"Update task",			// Name (debugging purposes)
		10000,					// Stack size in bytes
		NULL,					// Parameters to pass
		11,						// Priority
		NULL					// Handler
	);

	xTaskCreate(
		timeCounterTask,
		"Time counter",
		10000,
		NULL,
		20,
		NULL
	);
}



void loop() {
	SERVER_cleanup();
	//Serial.println("Here");
	vTaskDelay(1000 / portTICK_PERIOD_MS);

}


void error(uint8_t reboot){

    configASSERT(0);
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