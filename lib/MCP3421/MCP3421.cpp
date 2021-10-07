
/************************************************************************************
 * @version 3
 * @author 9morano
 * @date 30.09.2021
 * @todo 
 ***********************************************************************************/
#include "MCP3421.h"
#include <Wire.h>


/*---------------------------------------------------------------------------------*/
MCP3421::MCP3421()
{

}

/*---------------------------------------------------------------------------------*/
void MCP3421::setup(void)
{
    if(!Wire.begin(SDA_PIN, SCL_PIN)){
        Serial.println("I2C error on MCP3421");
    }

    // Address may be 0x68 or 0x69 ... check it automaticaly
    Wire.beginTransmission(MCP3421_ADDRESS);
    uint8_t error = Wire.endTransmission();

    if(error == 0){
        write(_config_register);
    }
    else{
        MCP3421_ADDRESS = 0x68;
        write(_config_register);
    }
}

/*---------------------------------------------------------------------------------*/
void MCP3421::setConfig(uint8_t conf_reg)
{
    _config_register = conf_reg;
    write(conf_reg);
}

/*---------------------------------------------------------------------------------*/
void MCP3421::getConfig(uint8_t *conf_reg)
{
    uint8_t tmp[3];
    read(tmp);
    // Last byte from the device is always config register
    *conf_reg = tmp[2];
}

/*---------------------------------------------------------------------------------*/
void MCP3421::startOneshot(void)
{
    write(_config_register | MCP3421_READY_BIT);
}

/*---------------------------------------------------------------------------------*/
uint8_t MCP3421::getAdcRaw(int16_t *val)
{
    uint8_t result[3];
    do{
        if(!read(result)){
            return 0;
        }
        *val = ((int16_t) result[0] <<8) | result[1];
    }
    // Repeat util READY bit is set to 0 --> indicates new value
    while((result[2] & MCP3421_READY_BIT) != 0x00);

    return 1;
}

/*---------------------------------------------------------------------------------*/
uint8_t MCP3421::checkAdcRaw(int16_t *val)
{
    uint8_t result[3];

    if(!read(result)){
        return 0;
    }

    *val = ((int16_t) result[0] <<8) | result[1];

    // Check if READY bit is set to 0 --> indicates new value
    if((result[2] & MCP3421_READY_BIT) != 0x00){
        return 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------------*/
void MCP3421::getAdc(float *val)
{
    // LSB for 16bit is 62.5uV
    // If you get 00000000 00000010, than you have 125uV
    // Which you have to divide with gain - in our case 8
    int16_t _val;

    getAdcRaw(&_val);

    *val = (_val * 62.5) / 8;
}

/*---------------------------------------------------------------------------------*/
void MCP3421::getPowa(float *power)
{
    // With our solar cell, 100uV represents 1W
    // If you get 350uV you have 3.5W
    float _voltage;

    getAdc(&_voltage);

    *power = _voltage / 100;

    // Cell correction 
    // Under reference we measured 1051W intead of 1000W
    *power = *power * 0.9514748;

    // Because we turned our cells the wrong way, values are inverted :D
    *power = abs(*power);
}

/*---------------------------------------------------------------------------------*/
/* ------------------------------- PRIVATE --------------------------------------- */
/*---------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------*/
void MCP3421::write(uint8_t _config){
    Wire.beginTransmission(MCP3421_ADDRESS);
    Wire.write(_config_register);
    Wire.endTransmission();
}

/*---------------------------------------------------------------------------------*/
uint8_t MCP3421::read(uint8_t _buff[])
{
    uint8_t i = 0;

    //Wire.beginTransmission(MCP3421_ADDRESS);
    Wire.requestFrom(MCP3421_ADDRESS, 3);
    while(Wire.available()){
        _buff[i] = Wire.read();
        i++;
    }
    if(i != 3){
        Serial.print("MCP3421 read error ");
        Serial.println(i);
        return 0;
    }
    Wire.endTransmission(); 
    return 1;
}













