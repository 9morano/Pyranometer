/************************************************************************************
 * @version 3
 * @author 9morano
 * @date 30.09.2021
 * @todo Check the hostname setting - this IP usage is not practical 
 ***********************************************************************************/

#include "Arduino.h"
#include "pyrano_wifi.h"

/*---------------------------------------------------------------------------------*/
uint8_t pyranoWIFI::init()
{
    // Has to be init in separate function, not already at constructor
    uint8_t status;
    IPAddress local_ip(192,168,6,1);
    IPAddress gateway(192,168,6,1);
    IPAddress subnet(255,255,255,0);

    status = w.softAPConfig(local_ip, gateway, subnet);

    //Serial.println(status);

    // Optional...default is max 19.5 dBm (check WiFiGeneric.h)
    //WiFi.setTxPower(44); // 11dBm

    // Optional...default is espressif
    //const char *hostname = "pyranometer";
    //status &= WiFi.softAPsetHostname(hostname);
    return status;
}

/*---------------------------------------------------------------------------------*/
uint8_t pyranoWIFI::APstart(void)
{
    is_on = 1;
    // Add the password parameter if you want the AP to be secured.
    return w.softAP(ssid);
}

/*---------------------------------------------------------------------------------*/
void pyranoWIFI::APstop(void)
{
    is_on = 0;
    w.softAPdisconnect(true);

}

/*---------------------------------------------------------------------------------*/
String pyranoWIFI::getIP(void)
{
    // Cast a IP address to string
    return w.softAPIP().toString();
}