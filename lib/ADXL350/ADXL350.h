#include "Arduino.h"

class ADXL350 {

    public:
        ADXL350();
        void begin();
        void setup();
        void setup(int16_t off_x, int16_t off_y, int16_t off_z);
        void getAccRaw(int16_t *x, int16_t *y, int16_t *z) ;
        void getAcc(float *x, float *y, float *z);
        void getInclination(float *pitch, float *roll);
        void getInclinationLPF(float *pitch, float *roll);

        void getRange(byte* rangeSetting);
        void setRange(int val);
        void setDataRate(int val);
        void printAllRegister();

        void calibrate();
        void autoCalibrate();

    private:
        byte _buff[6] ;		//	6 Bytes Buffer
        double _range_gain = 0.00390625;  // Default range

        void write(byte _address, byte _val);
        void read(byte _address, int _num, byte _buff[]);
        void setRegisterBit(byte regAdress, int bitPos, bool state);
	    bool getRegisterBit(byte regAdress, int bitPos);  

};
void print_byte(byte val);

/*************************** REGISTER MAP ***************************/
#define ADXL350_DEVID			0x00		// Device ID
#define ADXL350_RESERVED1		0x01		// Reserved. Do Not Access. 
#define ADXL350_THRESH_TAP		0x1D		// Tap Threshold. 
#define ADXL350_OFSX			0x1E		// X-Axis Offset. 
#define ADXL350_OFSY			0x1F		// Y-Axis Offset.
#define ADXL350_OFSZ			0x20		// Z- Axis Offset.
#define ADXL350_DUR				0x21		// Tap Duration.
#define ADXL350_LATENT			0x22		// Tap Latency.
#define ADXL350_WINDOW			0x23		// Tap Window.
#define ADXL350_THRESH_ACT		0x24		// Activity Threshold
#define ADXL350_THRESH_INACT	0x25		// Inactivity Threshold
#define ADXL350_TIME_INACT		0x26		// Inactivity Time
#define ADXL350_ACT_INACT_CTL	0x27		// Axis Enable Control for Activity and Inactivity Detection
#define ADXL350_THRESH_FF		0x28		// Free-Fall Threshold.
#define ADXL350_TIME_FF			0x29		// Free-Fall Time.
#define ADXL350_TAP_AXES		0x2A		// Axis Control for Tap/Double Tap.
#define ADXL350_ACT_TAP_STATUS	0x2B		// Source of Tap/Double Tap
#define ADXL350_BW_RATE			0x2C		// Data Rate and Power mode Control
#define ADXL350_POWER_CTL		0x2D		// Power-Saving Features Control
#define ADXL350_INT_ENABLE		0x2E		// Interrupt Enable Control
#define ADXL350_INT_MAP			0x2F		// Interrupt Mapping Control
#define ADXL350_INT_SOURCE		0x30		// Source of Interrupts
#define ADXL350_DATA_FORMAT		0x31		// Data Format Control
#define ADXL350_DATAX0			0x32		// X-Axis Data 0
#define ADXL350_DATAX1			0x33		// X-Axis Data 1
#define ADXL350_DATAY0			0x34		// Y-Axis Data 0
#define ADXL350_DATAY1			0x35		// Y-Axis Data 1
#define ADXL350_DATAZ0			0x36		// Z-Axis Data 0
#define ADXL350_DATAZ1			0x37		// Z-Axis Data 1
#define ADXL350_FIFO_CTL		0x38		// FIFO Control
#define ADXL350_FIFO_STATUS		0x39		// FIFO Status
