#include "Arduino.h"

class ADXL350 {

    public:
        ADXL350();
        void setup();
        void setup(int16_t off_x, int16_t off_y, int16_t off_z);     
        void begin();
        void standby();

        void getAccRaw(int16_t *x, int16_t *y, int16_t *z) ;
        void getAcc(float *x, float *y, float *z);
        void getInclination(float *pitch, float *roll);
        void getInclinationLPF(float *pitch, float *roll);

        void getRange(byte* rangeSetting);
        void setRange(int val);
        void setDataRate(int val);
        void enterSleepMode(void);
        void exitSleepMode(void);

        void calibrate();
        void autoCalibrate();
        void initInterupt(void);
        byte getInterupt(void);

        void printAllRegister();

    private:
        byte _buff[6] ;		//	6 Bytes Buffer
        double _range_gain = 0.00390625;  // Default range


        /* ACTIVITY INTERUPT CONFIG
         * Activity bit is set when value is larger than TRESH_ACT register
         * Threshold value is unsigned byte (8 bits) in scale of 31.2 mg/LSB
         */
        byte _activity_threshold = 0x64;    // 3g          

        /*
         * DOUBLE TAP INTERUPT CONFIG
         * Int is set when:
         * value greater than THRESH_TAP		(31.2 mg/LSB)
         * occurs les than DUR					(625 us/LSB)
         * with second tap after LATENT			(1.25 ms/LSB)	
         * but within the time in WINDOW		(1.25 ms/LSB)
         */ 
        byte _dt_threshold = 0x64;          // 3g       50
        byte _dt_duration = 0x10;           // 10ms     15
        byte _dt_latent = 0x64;             // 125ms    80
        byte _dt_window = 0x64;             // 125ms    100


        // I2C communication
        void write(byte _address, byte _val);
        void read(byte _address, int _num, byte _buff[]);
        void setRegisterBit(byte regAdress, int bitPos, bool state);
	    bool getRegisterBit(byte regAdress, int bitPos);  

        // Debug - print register values
        
        void printRegister(byte address);
        void printByte(byte val);
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