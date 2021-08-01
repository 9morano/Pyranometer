#include "SPIFFS.h"

class FileSystem {

    public:

        const char *current_file = "test.txt";

        void setup(void);

        void printInfo(void);
        void printFiles(void);
        void printMeasurementsFiles(void);

        uint8_t createFile(const char *path);
        void deleteFile(const char *path);
        void append(const char *path, const char *msg);
        
    private:

        const char *_measurement_dir = "/measure/";
        

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
 * 
 * 
 */




//uint8_t SERVER_check4html(void);



