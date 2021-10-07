#include <Arduino.h>
#include "project-config.h"
#include "ADXL350.h"

// FreeRTOS example
// a higher priority number means a task is more important (24 - most important)

// SUSPEND - suspend the task until it is resumed. Tasks can suspend each other
// BLOCK - block the task until some event (or timeout)...block with portMAX_DELAY basically suspends the task. Task can only block itself. DELAY is a cind of blocking
// DELETE - remove task from kernel management
// RESUME - resume suspended task (if resumed task has higher priority it will preemt the running task or else staied in ready state)

// If one task has while(1) loop, scheduler will use the other core for execution ... but watchdog will trigger exception and restart the esp32

TaskHandle_t task_led_handle = NULL;
TaskHandle_t task_core_handle = NULL;
TaskHandle_t task_count_handle = NULL;


// Toggle built in led
void toggleLED(void * parameter) {

	pinMode(LED_BUILTIN, OUTPUT);

	// Get parameter
	char *client_time;
	client_time = (char *) parameter;
  
	while(1){

		digitalWrite(LED_BUILTIN, HIGH);
		// Pause the task for 500ms
		vTaskDelay(500 / portTICK_PERIOD_MS);
		digitalWrite(LED_BUILTIN, LOW);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}


// Task that counts seconds
void count( void *param) {
	int counter = 0;

	while(1){
		Serial.print("Counter value: ");
		Serial.println(counter++);


		if(counter == 10 && task_led_handle != NULL && task_count_handle){
			Serial.println("Count task suspending led task");
      		vTaskSuspend(task_led_handle);
			vTaskSuspend(task_core_handle);
  		}
		
		if(counter == 15 && task_led_handle != NULL){
			Serial.println("Count task resuming led task");
      		vTaskResume(task_led_handle);
  		}

		// Delay fot 1 second
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}


// Task that only does one thing end exits
void onceTask(void *param) {

	Serial.println("Task that runs only once!");

	// Delete task - passing NULL will delete itself
	vTaskDelete(NULL);
}



// Task to check the core 
void checkCore(void *param) {
	while(1){
		Serial.print("Task running on core:");
		Serial.println(xPortGetCoreID());
		// Pause the task for 1000 ms
  		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

// Super important task
void superImportantTask(){
    vTaskSuspendAll();

    // Do something really important here
    // (Code that's timing sensitive and should
    // not be interrupted by FreeRTOS)

    // Resume all tasks again
    xTaskResumeAll();
}









void setup() {
	Serial.begin(SERIAL_BAUDRATE);
	delay(2000);

	

	
	xTaskCreate(
		toggleLED,		// Function
		"Toggle LED",	// Name (debugging purposes)
		1000,			// Stack size in bytes
		NULL,			// Parameters to pass
		1,				// Priority
		&task_led_handle			// Handler
	);

	xTaskCreate(onceTask, "Once", 500, NULL, 10, NULL);
	xTaskCreate(count, "Count", 1000, NULL,  1, &task_count_handle);

	// Create task pinned to core - additional (last) argument choses the core (0/1)
	xTaskCreatePinnedToCore(checkCore, "Core", 1000, NULL, 1, &task_core_handle, 0);


	// vTaskStartScheduler is automatically called here!
}

// Loop function is hooked to IDLE task - will run when CPU is IDLE
// It has lowest priority and will run only when CPU has no other things to do
// Should be called here and there to ensure deleted tasks free their memory (done here)
void loop() {
	//Serial.println("Loop");
	//vTaskDelay(100 / portTICK_PERIOD_MS);

	//Serial.println(uxTaskPriorityGet(NULL));

}