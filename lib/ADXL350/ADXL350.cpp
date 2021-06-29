/*
# After powerOn, device is in standby mode...measure mode - Bit D3 in register 0x2D (POWER_CTL)
# Recommended: configure device in standby mode....

*/


#include "ADXL350.h"
#include <Wire.h>

#define ADXL350_DEVICE      (0x53)    // Device Address for ADXL345
#define ADXL350_DATA_NUM    (6)       // Number of bytes to read (2 bytes per axis)

ADXL350::ADXL350()
{
	// Reset buff values
    for(int i=0; i<6; i++){
		_buff[i]=0;
	}
}

void ADXL350::setup()
{
	if(!Wire.begin()){
        Serial.println("I2C error...");
    }
	delay(100);

	// Go into standby mode
	write(ADXL350_POWER_CTL, 0);
	setRegisterBit(ADXL350_POWER_CTL, 3, false);

	// Disable interupts (set to default register values)
	write(ADXL350_INT_ENABLE, 0);
	write(ADXL350_INT_MAP, 0);

	// TODO delete below
	byte tmp;
	read(ADXL350_POWER_CTL, 1, &tmp);
	Serial.println(tmp);

	Serial.println("All register values");
	printAllRegister();
}

void ADXL350::setup(int16_t off_x, int16_t off_y, int16_t off_z)
{	
	if(!Wire.begin()){
        Serial.println("I2C error...");
    }
	delay(100);

	// Go into standby mode
	write(ADXL350_POWER_CTL, 0);
	setRegisterBit(ADXL350_POWER_CTL, 3, false);

	// Disable interupts (set to default register values)
	write(ADXL350_INT_ENABLE, 0);
	write(ADXL350_INT_MAP, 0);

	// Set offset values
	write(ADXL350_OFSX, off_x);
	write(ADXL350_OFSX, off_y);
	write(ADXL350_OFSX, off_z);
}

void ADXL350::begin()
{
    // Wakeup, Begin MEASURE, AUTO SLEEP of, ...
    write(ADXL350_POWER_CTL, 8);
}

void ADXL350::getAccRaw(int16_t *x, int16_t *y, int16_t *z)
{
    // Read Accel Data from ADXL345 (registers 0x32 through 0x37)
	read(ADXL350_DATAX0, ADXL350_DATA_NUM, _buff);	

	// Form 2 Bytes from buffer data
	*x = (((int16_t) _buff[1]) << 8) | _buff[0];   
	*y = (((int16_t) _buff[3]) << 8) | _buff[2];
	*z = (((int16_t) _buff[5]) << 8) | _buff[4];
}

void ADXL350::getAcc(float *x, float *y, float *z)
{
	int16_t _x = 0, _y = 0, _z = 0;

    // Read Accel Data from ADXL345 (registers 0x32 through 0x37)
	read(ADXL350_DATAX0, ADXL350_DATA_NUM, _buff);	

	// Form 2 Bytes from buffer data
	_x = (((int16_t) _buff[1]) << 8) | _buff[0];   
	_y = (((int16_t) _buff[3]) << 8) | _buff[2];
	_z = (((int16_t) _buff[5]) << 8) | _buff[4];

	*x = _x * _range_gain;
	*y = _y * _range_gain;
	*z = _z * _range_gain;
}

void ADXL350::getInclination(float *pitch, float *roll)
{
	float _x, _y, _z;
	getAcc(&_x, &_y, &_z);

	// Calculate roll and pitch (rotation around X-axis and Y-axis)
	// https://www.nxp.com/files-static/sensors/doc/app_note/AN3461.pdf
	*roll = atan(_y / sqrt(pow(_x, 2) + pow(_z, 2))) * 180 / PI;
	*pitch = atan(-1 * _x / sqrt(pow(_y, 2) + pow(_z, 2))) * 180 / PI;
}

// Smoother results (insensitive to shaking), but slower response
// Has to be called periodically
// Variables must be init with 0!
void ADXL350::getInclinationLPF(float *pitch, float *roll)
{
	float p, r;

	getInclination(&p, &r);

	// Low Pass Filter (96% old value and 6% new)
	*pitch = 0.94 * *pitch + 0.06 * p;
	*roll = 0.94 * *roll + 0.06 * r;
}

void ADXL350::getRange(byte* rangeSetting)
{
	byte _b;
	read(ADXL350_DATA_FORMAT, 1, &_b);
	*rangeSetting = _b & B00000011;
}

void ADXL350::setRange(int val)
{
	byte _s, _b;
	
	switch (val) {
		case 1:
			_range_gain = 0.001953125;
			_s = B00000000; 
		case 2:  
			_range_gain = 0.00390625;
			_s = B00000000; 	// TODO: popravi na 01
			break;
		case 4:  
			_range_gain = 0.0078125;
			_s = B00000001; 	// TODO: popravi na 10
			break;
		case 8:  
			_range_gain = 0.015625;
			_s = B00000010; 	// TODO: popravi na 11
			break;
		case 16: 
			_s = B00000011; 	// TODO: zbrisi ven
			break;
		default: 
			return;
	}
	read(ADXL350_DATA_FORMAT, 1, &_b);
	_s |= (_b & B11101100);
	write(ADXL350_DATA_FORMAT, _s);
}


// Lower datarate, lower consumption. 
void ADXL350::setDataRate(int val)
{
	byte _s, _b;

	switch(val){
		case 400:
			_s = B00001100;
			break;
		case 200:
			_s = B00001011;
			break;
		case 100:
			_s = B00001010;
			break;
		case 50:
			_s = B00001001;
			break;
		case 25:
			_s = B00001000;
			break;
		case 12: 	//12.5
			_s = B00000111;
			break;
		default:	// 100
			_s = B00001010;
			break;
	}
	read(ADXL350_BW_RATE, 1, &_b);
	_s |= (_b & B00010000);
	write(ADXL350_BW_RATE, _s);
}

// Print Register Values to Serial Output =
// Can be used to Manually Check the Current Configuration of Device
void ADXL350::printAllRegister()
{
	byte _b;
	Serial.print("0x00: ");
	read(0x00, 1, &_b);
	print_byte(_b);
	Serial.println("");
	int i;
	for (i=29;i<=57;i++){
		Serial.print("0x");
		Serial.print(i, HEX);
		Serial.print(": ");
		read(i, 1, &_b);
		print_byte(_b);
		Serial.println("");    
	}
}

void print_byte(byte val)
{
	int i;
	Serial.print("B");
	for(i=7; i>=0; i--){
		Serial.print(val >> i & 1, BIN);
	}
}



/* --------------------------------------------------------------------- */
/* -------------------------- PRIVATE ---------------------------------- */
/* --------------------------------------------------------------------- */

void ADXL350::write(byte _address, byte _val)
{
    Wire.beginTransmission(ADXL350_DEVICE); 
	Wire.write(_address);             
	Wire.write(_val);                 
	Wire.endTransmission();  
}

void ADXL350::read(byte _address, int _num, byte _buff[])
{
	Wire.beginTransmission(ADXL350_DEVICE);  
	Wire.write(_address);             
	Wire.endTransmission();         
	
	Wire.beginTransmission(ADXL350_DEVICE); 
	Wire.requestFrom(ADXL350_DEVICE, _num);
	
	int i = 0;
	while(Wire.available())					
	{ 
		_buff[i] = Wire.read();
		i++;
	}
	if(i != _num){
        Serial.println("Read error!");
	}
	Wire.endTransmission();         	
}


void ADXL350::setRegisterBit(byte regAdress, int bitPos, bool state)
{
	byte _b;
	read(regAdress, 1, &_b);
	if (state) {
		_b |= (1 << bitPos);  // Forces nth Bit of _b to 1. Other Bits Unchanged.  
	} 
	else {
		_b &= ~(1 << bitPos); // Forces nth Bit of _b to 0. Other Bits Unchanged.
	}
	write(regAdress, _b);  
}

bool ADXL350::getRegisterBit(byte regAdress, int bitPos)
{
	byte _b;
	read(regAdress, 1, &_b);
	return ((_b >> bitPos) & 1);
}


/* --------------------------------------------------------------------- */
/* -------------------- APPLICATION SPECIFIC --------------------------- */
/* --------------------------------------------------------------------- */

// Function to get offset data 
void ADXL350::calibrate()
{	
	/* Procedure to calibrate the ADXL sensor:
	 * -----------------------------------------------------------------
	 * 1) Place sensor on straight surface
	 * 2) Run the while loop below
	 * 3) Get average measurement of one axis 		[avg = ~283]
	 * 4) Get err value: err = avg - 256 			[err = 27]
	 * 5) Calculate offset: off = - round(err/4) 	[off = -7 LSB]
	 * 6) Off sign must be opposite from err sign. 
	 * 7) Write the off value into appropriate register.
	 * NOTE: Calibration offset is not permanently stored into registers!
	 */

	int16_t x, y, z;
	int32_t avg_x = 0, avg_y = 0, avg_z = 0;
	int err_x, err_y, err_z;
	int offset_x, offset_y, offset_z;

	// Set data rate to 100 Hz
	setDataRate(100);
	// Set range to +-1g 
	setRange(2); // TODO - nastavi na 1

	Serial.println("Calibrating ADXL350 device...to exit, coment out the function 'calibrate()'");
	
	while(1){
		Serial.println("Put device in desired position!");
		Serial.println("Obtaining data ...");
		for(uint8_t i=0; i<100; i++){
			getAccRaw(&x, &y, &z);
			avg_z += z;
			avg_x += x;
			avg_y += y;
			delay(10);
		}
		avg_z = avg_z / 100;
		avg_x = avg_x / 100;
		avg_y = avg_y / 100;

		err_z = avg_z - 256;	// TODO - nastavi na 512
		err_x = avg_x - 256;	// TODO - nastavi na 512
		err_y = avg_y - 256;	// TODO - nastavi na 512
		offset_z = round(err_z/4);
		offset_x = round(err_x/4);
		offset_y = round(err_y/4);
		offset_z *= -1;
		offset_x *= -1;
		offset_y *= -1;

		Serial.print("Calculated offsets:  X=");
		Serial.print(offset_x);
		Serial.print(" Y=");
		Serial.print(offset_y);
		Serial.print(" Z=");
		Serial.println(offset_z);
		delay(1000);
	}
}



void ADXL350::autoCalibrate()
{
	int16_t x, y, z;
	int32_t avg = 0;
	int err, offset;

	// Set data rate to 100 Hz
	setDataRate(100);
	// Set range to +-1g 
	setRange(2); // TODO - nastavi na 1


	/* --------------------- Z-axis ------------------------------------ */

	Serial.println("Put the device in horizontal orientation!");

	// TODO while(){ počaki na potrditev uporabnika }

	// Get 100 raw measurements
	for(uint8_t i=0; i<100; i++){
		getAccRaw(&x, &y, &z);
		avg += z;
		delay(10);
	}
	avg = avg / 100;

	Serial.print("Average z = ");
	Serial.println(avg);

	err = avg - 256;	// TODO - nastavi na 512
	offset = round(err/4);
	offset *= -1;

	Serial.print("Calculated z offset = ");
	Serial.println(offset);

	write(ADXL350_OFSZ, offset);

	/* --------------------- X-axis ------------------------------------ */
	Serial.println("Put the device in horizontal orientation!");

	// TODO while(){ počaki na potrditev uporabnika }

	// Get 100 raw measurements
	for(uint8_t i=0; i<100; i++){
		getAccRaw(&x, &y, &z);
		avg += x;
		delay(10);
	}
	avg = avg / 100;

	Serial.print("Average x = ");
	Serial.println(avg);

	err = avg - 256;	// TODO - nastavi na 512
	offset = round(err/4);
	offset *= -1;

	Serial.print("Calculated x offset = ");
	Serial.println(offset);

	write(ADXL350_OFSX, offset);

	/* --------------------- Y-axis ------------------------------------ */
	Serial.println("Put the device in horizontal orientation!");

	// TODO while(){ počaki na potrditev uporabnika }

	// Get 100 raw measurements
	for(uint8_t i=0; i<100; i++){
		getAccRaw(&x, &y, &z);
		avg += y;
		delay(10);
	}
	avg = avg / 100;

	Serial.print("Average y = ");
	Serial.println(avg);

	err = avg - 256;	// TODO - nastavi na 512
	offset = round(err/4);
	offset *= -1;

	Serial.print("Calculated y offset = ");
	Serial.println(offset);

	write(ADXL350_OFSY, offset);
}
