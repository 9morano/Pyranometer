/************************************************************************************
 * Library for analog to digital converter MCP3421 via I2C interface.
 * 
 * Only one register on the device and its default values:
 *  +---+---+---+---+---+---+---+---+
 *  |RDY|C1 |C0 |O/C|S1 |S0 |G1 |G0 |
 *  +---+---+---+---+---+---+---+---+
 *  | 1 | x | x | 1 | 0 | 0 | 0 | 0 |
 *  +---+---+---+---+---+---+---+---+
 * 
 * RDY      - ready bit
 * O/C      - one shot(0) or continuous operation (1)
 * S1 & S0  - sample rate
 *      00 = 240  SPS (12bit)   (LSB = 1mV)
 *      01 = 60   SPS (14bit)   (LSB = 250uV)
 *      10 = 15   SPS (16bit)   (LSB = 62.5uV)
 *      11 = 3.75 SPS (18bit)   (LSB = 15.625uV)
 * 
 * G1 & G0  - gain selection
 *      00 = 1x     
 *      01 = 2x     
 *      10 = 4x     
 *      11 = 8x     
 * 
 * 
 * @version 3
 * @author 9morano
 * @date 30.09.2021
 * @note For pyranometer, we want: 
 *          0001 1011 --> default value of config_register
 *          Single shot with max gain and 16bit resolution
 * 
 *          Conversions from ADC value to voltage (getAdc()) and power (getPowa()) 
 *          are costum functions for this setup (= pyranometer) only!
 ***********************************************************************************/

#include "Arduino.h"

/*---------------------------------------------------------------------------------*/
#define SDA_PIN				(26) 			
#define SCL_PIN				(27)
uint8_t MCP3421_ADDRESS	 = 0x69;

/*---------------------------------------------------------------------------------*/
/*---------------------------------- ADC CLASS ------------------------------------*/
/*---------------------------------------------------------------------------------*/
class MCP3421 {

    public:
        MCP3421();

        /**
         * Initializes I2C connection. SDA and SCL pines are defined in MCP3421.h
         * 
         * @note Address may be 0x68 or 0x69..function will check both
         */
        void setup(void);

        /**
         * Sets configuration register (class variable) and writes it to the device
         * 
         * @param conf_reg configuration register value to write
         */
        void setConfig(uint8_t conf_reg);

        /**
         * Optain current configuration register
         * 
         * @param conf_reg pointer to store the value
         */
        void getConfig(uint8_t *conf_reg);

        /**
         * Starts one shot conversion (single shot)
         */
        void startOneshot(void);

        /**
         * BLOCKING MODE - waits until raw ADC value is ready
         * 
         * @param val pointer to store the value
         */
        uint8_t getAdcRaw(int16_t *val);

        /**
         * NON BLOCKING MODE - check for raw ADC only once
         * 
         * @param val pointer to store the value
         * @return 1 if success, 0 otherwise
         */
        uint8_t checkAdcRaw(int16_t *val);

        /**
         * BLOCKING MODE - waits until raw ADC value is ready, then transforms it into voltage
         * 
         * @param val pointer to store the value
         */
        void getAdc(float *val);

        /**
         * BLOCKING MODE - waits until raw ADC value is ready, then transforms it into power
         * 
         * @param val pointer to store the value
         */
        void getPowa(float *power);


    private:
        // Single shot with max gain and 16bit resolution
        uint8_t _config_register = B00011011;

        /**
         * Writes into config register - you can only write the config register
         */
        void write(uint8_t _config);

        /**
         * Read 3 bytes - last one is always config register.
         * 
         * @param _buff array to store the register value
         * @warning If resolution is set to max 18 bit, then the devices returns 4 bytes!
         */
        uint8_t read(uint8_t _buff[]);


};

#define MCP3421_READY_BIT       (0x80)

