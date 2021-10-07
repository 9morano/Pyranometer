/************************************************************************************
 * This is a costum library for ESP32 file system access...based on SPIFFS.h. Used
 * for storing measurement on the flash of the device for longer time.
 * 
 * Name of any file can be max 32 chars long!
 * 
 * Measurements are stored in folder called /measure/ ... any changes must be fixed in
 * the server script as well. 
 * To create measurement file, just call function createFile(name_of_file), which will
 * create file called /measure/name_of_file.txt. If this name is already taken, it will
 * append the data to existing file. Could be upgraded ...
 *
 * To store with space, measurement are stored in CSV format. Check the functions for
 * details.
 * 
 * @version 3
 * @author 9morano
 * @date 30.09.2021
 * @note 
 ***********************************************************************************/


#include "SPIFFS.h"

/*---------------------------------------------------------------------------------*/
/*----------------------------- FILE SYSTEM CALSS ---------------------------------*/
/*---------------------------------------------------------------------------------*/
class FileSystem {

    public:

        // Variable to store the current filename
        const char *current_file = "test.txt";

        /**
         * Initialize SPIFSS
         */
        void setup(void);
        
        /**
         * Print filesystem info, accessable files and used place
         */
        void printInfo(void);

        /**
         * Print all accessable files
         */
        void printFiles(void);

        /**
         * Print all accessable measurement files
         */
        void printMeasurementsFiles(void);

        /**
         * Delete a file
         * 
         * @param filename the name of the file to delete
         */
        void deleteFile(const char *path);

        /**
         * Create new measurement file. Automatically appends the appendix .csv and 
         * prepends the folder name. It is using strNcat to avoid overflow.
         * It automatically changes the current filename variable to follow the name.
         * 
         * @param filename name to store the file (max length 12 chars! Others will be deleted)
         * @return 1 if success
         */
        uint8_t createFile(const char *filename);

        /**
         * Change current filename variable - used in global context to follow the current file.
         * 
         * @param filename
         */
        uint8_t changeCurrentFilename(const char *filename);

        /**
         * Function to format the String in CSV and store the measurements
         * 1999.9,24.0,00.5,35
         */
        uint8_t storeMeasurement(float *power, float *pitch, float *roll, uint8_t *temp);

        /**
         * Same as storeMeasurement but adds the timestamp to the line in a file
         * 123015,1999.9,24.0,00.5,35
         */
        uint8_t storeMeasurementWTimstamp(float *power, float *pitch, float *roll, uint8_t *temp, uint8_t *h, uint8_t *m, uint8_t *s);



    private:

        // Name of measurement directory
        const char *_measurement_dir = "/measure/";

        // Variable to store the current measurement filename
        char _current_filename[31] = "";
        
        /**
         * Add line to file.
         * 
         * @param path path of file
         * @param msg line to store
         */
        uint8_t append(const char *path, const char *msg);
};
