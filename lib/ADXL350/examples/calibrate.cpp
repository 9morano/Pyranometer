#include <Arduino.h>
#include "Wire.h"
#include "ADXL350.h"
#include "project-config.h"

// Calculated offset for prototrip board
#define ACC_OFFSET_X                (-3)
#define ACC_OFFSET_Y                (0)
#define ACC_OFFSET_Z                (12)

ADXL350 adxl = ADXL350();

void setup() {
	Serial.begin(SERIAL_BAUDRATE); 

	delay(2000);

	// 1) First calibrate device in the case / box 
	//adxl.calibrate();

	// 2) Put measured values into the setup function
	//adxl.setup(7, -2, 5);

	// 3) You can use auto test function
	//adxl.autoCalibrate();


	// Example of an prototip board
	adxl.setup(ACC_OFFSET_X, ACC_OFFSET_Y, ACC_OFFSET_Z);
	adxl.setRange(1);
	adxl.setDataRate(100);

	Serial.println("All register values:");
	adxl.printAllRegister();

	adxl.begin();

	delay(1000);
}



int16_t x,y,z; 
float pitch = 0, roll = 0;

void loop() {

	/*
	// Get acceleration
	adxl.getAccRaw(&x, &y, &z);
	Serial.print(x);
	Serial.print(", ");
	Serial.print(y);
	Serial.print(", ");
	Serial.println(z); 
	*/


	// Get inclination
	adxl.getInclination(&pitch, &roll);
	Serial.print("Pitch and roll: ");
	Serial.print(pitch);
	Serial.print(" ");
	Serial.println(roll);


	delay(100);
}
