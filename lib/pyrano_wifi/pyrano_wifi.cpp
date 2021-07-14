/* --------------------------------------------------------------------------------------------------
 * TODO
 * 
 * Ver.:  2
 * Auth.: Grega Morano
 * Date : 24.05.2021
 * About:
 * 
 * --------------------------------------------------------------------------------------------------*/

#include "Arduino.h"
#include "pyrano_wifi.h"

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

uint8_t pyranoWIFI::APstart(void)
{
    is_on = 1;
    // Remove the password parameter if you want the AP to be open.
    return w.softAP(ssid, password);
}

void pyranoWIFI::APstop(void)
{
    is_on = 0;
    w.softAPdisconnect(true);

}

String pyranoWIFI::getIP(void)
{
    // Cast a IP address to string
    return w.softAPIP().toString();
}