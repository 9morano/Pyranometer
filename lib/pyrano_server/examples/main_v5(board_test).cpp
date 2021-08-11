
#include <Arduino.h>
#include "project-config.h"
#include "ADXL350.h"
#include "MCP3421.h"


// Create objects
ADXL350 adxl = ADXL350();	// adxl to controll the accelerometer
MCP3421 adc = MCP3421();	// adc to controll the ADC



void setup() {
	// Defined in project-conf.h
	Serial.begin(SERIAL_BAUDRATE);
	delay(2000);

	// ADC setup
	adc.setup();

	uint8_t c=0;
	adc.getConfig(&c);
	Serial.print("ADC config reg must be 27 (or 155) = ");
	Serial.println(c);


	// ACC setup
	adxl.setup();
	adxl.setRange(1);
  	adxl.setDataRate(25);
	adxl.printAllRegister();
	adxl.begin();
}



void loop() {
	int16_t aval = 0;
	float voltage = 0, power = 0;


	adc.startOneshot();
	adc.getAdcRaw(&aval);

	voltage = (aval * 62.5)/8;
	power = voltage/100;

	Serial.print("ADC: ");
	Serial.print(aval);
	Serial.print(" --> Measured voltage (uV)= ");
	Serial.print(voltage);
	Serial.print(" and power = ");
	Serial.println(power);


	float pitch = 0, roll = 0;
	adxl.getInclination(&pitch, &roll);

	Serial.print("Pitch'n Roll: ");
	Serial.print(pitch);
	Serial.print(" | ");
	Serial.println(roll);


	vTaskDelay(500 / portTICK_PERIOD_MS);
}