/************************************************************************************
 * This is a costum library for Pyrano WiFi access point configuration. Used only for
 * setting up the AP and shutting it down when needed.
 * 
 * The default values are:
 *      IP: 192.168.6.1
 *      GATEWAY: 192.168.6.1
 *      MASK: 255.255.255.0
 *      SSID: Pyranometer
 *      PASSWORD: /
 *      TX_POWER: max
 * 
 * @version 3
 * @author 9morano
 * @date 30.09.2021
 * @note 
 ***********************************************************************************/

#include <WiFi.h>

class pyranoWIFI {
    private:
        WiFiAPClass w;
        const char *ssid = "Pyranometer";
        const char *password = "123456789"; //Pass must be higher than 8 characters

    public:
        // Variable to store the state of the WiFi interface
        uint8_t is_on = 0;

        /**
         * Initializes the WiFi access point interface with default config:
         *      IP: 192.168.6.1
         *      GATEWAY: 192.168.6.1
         *      MASK: 255.255.255.0
         */
        uint8_t init(void);

        /**
         * Start the WiFi access point
         * Optionali add the password parameter to make the AP secure.
         */
        uint8_t APstart(void);

        /**
         * Stop the WiFi interface
         * @todo Check if the radio module goes down as wel...
         */
        void    APstop(void);

        /**
         * Return the IP address of the device
         */
        String  getIP(void);
};