/************************************************************************************
 * Library for 3-axis accelerometer ADXL350 --> using I2C communication.
 * Product site: https://www.analog.com/en/products/adxl350.html#product-overview
 * Ultra low power. user selectable range (+-1, +-2, +-4, +-8 g), sample rate, ...
 * 
 * @version 3
 * @author 9morano
 * @date 30.09.2021
 * @note Configure device while in stand by mode, only then enter measurement 
 *       mode with begin()
 ***********************************************************************************/

#include "Arduino.h"

/*---------------------------------------------------------------------------------*/
/*----------------------------- REGISTER MAP --------------------------------------*/
/*---------------------------------------------------------------------------------*/
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


/*---------------------------------------------------------------------------------*/
#define ADXL350_ADDRESS     (0x53)      // Device Address for ADXL350
#define ADXL350_DATA_NUM    (6)         // Number of bytes to read (2 bytes per axis)
#define SDA_PIN				(26) 			
#define SCL_PIN				(27)	


/*---------------------------------------------------------------------------------*/
/*----------------------------- ACCELEOMETER CLASS --------------------------------*/
/*---------------------------------------------------------------------------------*/
class ADXL350 {

    public:
        ADXL350();

        /**
         * Initializes I2C connection. SDA and SCL pines are defined in ADXL350.h
         */
        void setup();

        /**
         * Same as setup(), with added measurement offset correction
         * 
         * @param offset in all three axis
         */
        void setup(int16_t off_x, int16_t off_y, int16_t off_z);

        /**
         * Enter the measurement mode.
         */
        void begin();

        /**
         * Enter standby mode 
         *
         * @note low power (0.1uA) but no measurements are available and interupts are disabled
         */
        void standby();

        /**
         * Obtain measurement from device registers
         *
         * @param x,y,z pointers to the vars
         */
        void getAccRaw(int16_t *x, int16_t *y, int16_t *z);
        
        /**
         * Get acceleration in each axis in terms of g-force
         * 
         * @param x,y.z
         */
        void getAcc(float *x, float *y, float *z);

        /**
         * Calculate roll and pitch - rotation around X and Y-axis
         * 
         * @param pitch - rotation around X-axis
         * @param roll - rotation around Y-axis
         * @see https://www.nxp.com/files-static/sensors/doc/app_note/AN3461.pdf
         */
        void getInclination(float *pitch, float *roll);

        /**
         * Get the roll and pitch with added low pass filter.
         * Smoother results (insensitive to shaking), but slower response
         * 
         * @param pitch - rotation around X-axis
         * @param roll - rotation around Y-axis
         * @note must be called periodically
         */
        void getInclinationLPF(float *pitch, float *roll);

        /**
         * Set the measurement range.
         * 
         * @param val - possible range: +-1g, +-2g, +-4g, +-8g
         */
        void setRange(int val);
        void getRange(byte* rangeSetting);

        /**
         * Set the data rate of measurement sampling.
         * 
         * @param val - possible datarates: 12, 25, 50, 100, 200, 400
         * @note Lower datarate --> lower consumption
         */
        void setDataRate(int val);

        /**
         * Put device in sleep mode - power saving (40uA)
         * 
         * @note lower sampling rate, but activity interupt can be used
         */
        void enterSleepMode(void);

        /**
         * Exit sleep mode.
         */
        void exitSleepMode(void);


        /**
         * Initialize 'activity' interupt.
         * Threshold is defined as _activity_threshold in ADXL350.h
         * Mapped to INT2 --> routed to GPIO14
         * 
         * @warning Interupt bit is cleared when SOURCE register is read
         */
        void initActivityInt(void);

        /**
         * Read the interupt source and return 1 if activity detected.
         * 
         * @return 1 if activity detected, 0 otherwise
         */
        byte getActivity(void);


        /**
         * Initialize 'activity' and 'double tap' interupts
         * 
         * @note not tested yet
         */
        void initInterupt(void);

        /**
         * Get the interupt source
         * 
         * @return 16 means activity, 32 means double tap
         */
        byte getInterupt(void);
        
        /**
         * Print all register values to serial output.
         */
        void printAllRegister();


        void calibrate();
        void autoCalibrate();





    private:
        byte _buff[6];
        double _range_gain = 0.00390625;


        /* ACTIVITY INTERUPT CONFIG
         * Activity bit is set when value is larger than TRESH_ACT register
         * Threshold value is unsigned byte (8 bits) in scale of 31.2 mg/LSB
         */
        byte _activity_threshold = 0x32;    // 3g          

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


        /**
         * Write a register over I2C.
         * 
         * @param _address - addres of the register to write
         * @param _val - value to send
         */
        void write(byte _address, byte _val);

        /**
         * Read a register over I2C.
         * 
         * @param _address - address of the register
         * @param _num - number of bytes to read
         * @param _buff - array to store read data
         * @return 1 if success, 0 otherwise
         */
        byte read(byte _address, int _num, byte _buff[]);

        /**
         * Set one bit in the register, while storing others.
         */
        void setRegisterBit(byte regAdress, int bitPos, bool state);

        /**
         * Get register bit.
         */
	    bool getRegisterBit(byte regAdress, int bitPos);  
       
        /**
         * Prints register value - debug purposes.
         */
        void printRegister(byte address);

        void printByte(byte val);
};
