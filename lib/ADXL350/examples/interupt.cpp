/* Poizkus igranja z interupti - activity in double tap
 * Interupt sproži rutino ADXL_ISR ... v njej bi morali pobrisati interupt pin, kar lahko nardimo s prebranim
 * SOURCE registrom. Tega pa ne moremo narediti v ISR rutini, ker je I2C prepočasen.
 * Workaround: v ISR kliči task, ki prebere registre na pospeškometru.
 */

#include <Arduino.h>
#include "Wire.h"
#include "ADXL350.h"

ADXL350 adxl = ADXL350();

#define adxl_int_pin 18

TaskHandle_t accTaskHandle = NULL;


// IRAM_ATTR - attribute that instructs ESP32 to put the function into internal RAM
void ADXL_ISR(){
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;;

	// TODO clear interupt bit ... how to?

	xTaskNotifyFromISR( accTaskHandle, 0, eNoAction, &xHigherPriorityTaskWoken);

	// Request context switch to the task, if new task has higher priority
	if(xHigherPriorityTaskWoken){
		portYIELD_FROM_ISR();
	}
}

// FreeRTOS task
void accTask(void *param) {

	byte interupt;

	while(1){
		/* Waiting for notifications, blocking indefinitely (no timeout, so there is no need to check the function return value). 
		Different bits in the notification value of other tasks or interrupt settings represent different events. Parameter 0x00 indicates 
		that the notification value bit of the task is not cleared before the notification is used. The parameter ULONG_MAX indicates that 
		the function notification value is set to 0 before the function xTaskNotifyWait() exits.*/
        xTaskNotifyWait( 0x00, ULONG_MAX, NULL, portMAX_DELAY);

		adxl.setup();
		interupt = 0;
		Serial.print("Interupt triggered: ");
		interupt = adxl.getInterupt();

		switch(interupt){
			case 1:
				Serial.println("Activity");
				break;
			case 2:
				Serial.println("Double tap");
				break;
			default:
				Serial.print(interupt);
				Serial.println("unknown");
				break;
		}
		adxl.begin();
	}
}



void setup() {
	Serial.begin(9600); 
	delay(100);

	xTaskCreate(
        accTask,    		// function name
        "Task 1", 			// task name for debugging
        1000,     			// stack depth in bytes
        NULL,     			// task parameters
        21,       		 	// priority, higest number is first
        &accTaskHandle      // task handle (to interact with task from outside)
  	);

	adxl.setup();
	adxl.setRange(1);

	Serial.println("All register values: ");
	adxl.printAllRegister();

    // Configure interupt
	adxl.initInterupt();
	pinMode(adxl_int_pin, INPUT_PULLUP);
	attachInterrupt(adxl_int_pin, ADXL_ISR, RISING);

	adxl.begin();

	delay(1000);
}

float pitch = 0, roll = 0;
byte interupt = 0;

void loop() {

	/*
    while(1){

		delay(10);
		if(digitalRead(adxl_int_pin) == HIGH){
			Serial.println("HI");

			Serial.print("Interupt triggered: ");
			interupt = adxl.getInterupt();
			switch(interupt){
				case 1:
					Serial.println("Activity");
					break;
				case 2:
					Serial.println("Double tap");
					break;
				default:
					Serial.print(interupt);
					Serial.println("unknown");
					break;
			}
			delay(1000);

		}
		else{
			Serial.println("LOW");
		}
	}*/


	adxl.getInclination(&pitch, &roll);
	Serial.print("Pitch and roll: ");
	Serial.print(pitch);
	Serial.print(" ");
	Serial.println(roll);


    delay(100);
}

