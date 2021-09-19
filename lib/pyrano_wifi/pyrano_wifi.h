/* --------------------------------------------------------------------------------------------------
 * TODO add #ifnotdefined
 * 
 * Ver.:  2
 * Auth.: Grega Morano
 * Date : 24.05.2021
 * About:
 * 
 * --------------------------------------------------------------------------------------------------*/

#include <WiFi.h>

class pyranoWIFI {
    private:
        WiFiAPClass w;
        const char *ssid = "Pyranometer";
        const char *password = "123456789"; //Pass must be higher than 8 characters

    public:
        uint8_t is_on = 0;
        uint8_t init(void);
        uint8_t APstart(void);
        void    APstop(void);
        String  getIP(void);
};