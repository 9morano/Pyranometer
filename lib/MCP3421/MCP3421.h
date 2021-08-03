
#include "Arduino.h"

class MCP3421 {

    public:
        MCP3421();
        void setup(void);
        void setConfig(uint8_t conf_reg);
        void getConfig(uint8_t *conf_reg);

        void startOneshot(void);
        uint8_t getAdcRaw(int16_t *val);
        uint8_t checkAdcRaw(int16_t *val);
        void getAdc(float *val);
        void getPowa(float *power);


    private:
        // Single shot with max gain and 16bit resolution
        uint8_t _config_register = B00011011;

        void write(uint8_t _config);
        uint8_t read(uint8_t _buff[]);


};

#define MCP3421_READY_BIT       (0x80)

/*  Only register on the device and its default values:
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
 * For pyranometer, we want: 
 * 0001 1011 --> default value of config_register
 * Single shot with max gain and 16bit resolution
 */ 