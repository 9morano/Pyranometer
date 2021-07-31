#include "SPIFFS.h"

class FileSystem {

    public:

        const char *current_file = "test.txt";

        uint8_t setup(void);

        void printInfo(void);
        void printFiles(void);
        void printMeasurementsFiles(void);

        void createDir(void);
        void createFile(const char *path);
        void deleteFile(const char *path);
        void append(const char *path, const char *msg);
        
    private:

        const char *_measurement_dir = "/measure/";
        

};


/* Name of a file can be max 32 chars long
 * Folder for measurements is called /measure/
 *
 * 
 * 
 */




//uint8_t SERVER_check4html(void);




