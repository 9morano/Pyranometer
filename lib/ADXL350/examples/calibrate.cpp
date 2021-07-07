#include <Arduino.h>
#include "Wire.h"
#include "ADXL350.h"

ADXL350 adxl = ADXL350();

void setup() {
	Serial.begin(9600); 

	delay(100);

	// TODO: First calibrate device in the case / box and put
	// measured values into the setup function

	//adxl.calibrate();
	//adxl.setup(7, -2, 5);

	// You can use auto test function
	//adxl.autoCalibrate();

	adxl.setup();
	adxl.setRange(1);

	Serial.println("All register values:");
	adxl.printAllRegister();

	adxl.begin();

	delay(1000);
}



float x,y,z; 
float pitch = 0, roll = 0;

void loop() {

    // Get acceleration
	adxl.getAcc(&x, &y, &z);
	Serial.print(x);
	Serial.print(", ");
	Serial.print(y);
	Serial.print(", ");
	Serial.println(z); 

	
/*
    // Get inclination
	adxl.getInclination(&pitch, &roll);
	Serial.print("Pitch and roll: ");
	Serial.print(pitch);
	Serial.print(" ");
	Serial.println(roll);
*/

delay(100);

}

