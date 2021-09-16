#include "SPIFFS.h"

class FileSystem {

    public:

        const char *current_file = "test.txt";

        void setup(void);

        void printInfo(void);
        void printFiles(void);
        void printMeasurementsFiles(void);

        void deleteFile(const char *path);
        uint8_t createFile(const char *path);
        uint8_t changeCurrentFilename(const char *filename);

        uint8_t append(const char *path, const char *msg);
        uint8_t storeMeasurement(float *power, float *pitch, float *roll, uint8_t *temp);
        uint8_t storeMeasurementWTimstamp(float *power, float *pitch, float *roll, uint8_t *temp, uint8_t *h, uint8_t *m, uint8_t *s);
        
    private:

        const char *_measurement_dir = "/measure/";
        char _current_filename[31] = "";
        

};


/* Name of a file can be max 32 chars long
 * Folder for measurements is called /measure/
 *
 * Files are stored automaticaly inside this folder...
 * To create measurement file, just call function createFile(name_of_file), which will
 * create file called /measure/name_of_file.txt. If this name is already taken, it will
 * try to create file with appendix to its filename...for example /measure/name_of_file_0.txt
 * If it doesn't succeed up to 5 times, function wil return false...new measurements will
 * overwrite the file with original filename (/measure/name_of_file.txt) - no safety here, no time for it :)
 */

