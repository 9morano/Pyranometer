/************************************************************************************
 * @version 3
 * @author 9morano
 * @date 30.09.2021
 * @todo Delete debug printf functions .. 
 ***********************************************************************************/

#include "pyrano_fs.h"

/*---------------------------------------------------------------------------------*/
void FileSystem::setup(void)
{
    if(!SPIFFS.begin()){
        Serial.println("Error while mounting SPIFFS");
    }
}

/*---------------------------------------------------------------------------------*/
void FileSystem::printInfo(void)
{
    uint32_t totalBytes = SPIFFS.totalBytes();
    uint32_t usedBytes = SPIFFS.usedBytes();

    Serial.println("===== File system info =====");
 
    Serial.print("Total space:      ");
    Serial.print(totalBytes);
    Serial.println("byte");
 
    Serial.print("Total space used: ");
    Serial.print(usedBytes);
    Serial.println("byte");

    Serial.println("===== Accessible files =====");
    printFiles();

}

/*---------------------------------------------------------------------------------*/
void FileSystem::printFiles(void)
{
    File root = SPIFFS.open("/");
    
    Serial.println("Files found:");
    while (true) {
 
        File entry =  root.openNextFile();

        if (! entry) {
            break;
        }
        Serial.println(entry.name());
        entry.close();
  }
}

/*---------------------------------------------------------------------------------*/
void FileSystem::printMeasurementsFiles(void)
{
    File measurements = SPIFFS.open("/measure");
    
    Serial.println("Measurements found: ");
    while(true){
        File file = measurements.openNextFile();
        
        if(!file){
            break;
        }

        Serial.print(file.name());
        Serial.print(" --> size:");
        Serial.println(file.size(), DEC);
        file.close();
    }
}

/*---------------------------------------------------------------------------------*/
void FileSystem::deleteFile(const char *path)
{
    Serial.printf("Deleting file: %s\r\n", path);
    if(!SPIFFS.remove(path)){
        Serial.println("- delete failed");
    }
}

/*---------------------------------------------------------------------------------*/
uint8_t FileSystem::createFile(const char *filename)
{
    char path[31] = "";
    char appendix[5] = ".csv";
    uint8_t success = 0;

     // path = /measure/filename.txt
    strncat(path, _measurement_dir, 10);
    strncat(path, filename, 12);
    strncat(path, appendix, 5);


    File f = SPIFFS.open(path, FILE_WRITE);
    if(!f){
        Serial.printf("Failed to create file %s \n", path);
    }
    else{
        Serial.printf("Created new measurements file: %s \n", path);
        f.print("Meritve:\n");
        success = 1;

    }

    strncpy(_current_filename, path, 31);
    return success;    
}

/*---------------------------------------------------------------------------------*/
/*
 * Gracefull attempt to store measurements in case the filename is the same...
 * Nevermind - stored here if someone will try to do the same :) 
uint8_t FileSystem::createFile(const char *filename)
{
    char path[30];
    char appendix[7] = ".csv";
    uint8_t success = 0;

    for(uint8_t i = 0; i<5; i++){
        // path = /measure/filename.txt
        strcat(path, _measurement_dir);
        strcat(path, filename);
        strcat(path, appendix);

        File check = SPIFFS.open(path, FILE_READ);

        // File with that name doesn't exists
        if(!check.available()){
            File f = SPIFFS.open(path, FILE_WRITE);
            if(!f){
                Serial.printf("Failed to create file %s \n", path);
            }
            else{
                Serial.printf("Created new measurements file: %s \n", path);
                f.print("Datoteka z meritvami");
                success = 1;
                break;
            }
        }

        Serial.printf("File %s allready exists..trying new one\n", path);
        // path = /measure/filename_i.txt
        path[0] = 0;
        sprintf(appendix, "_%d.csv", i);
    }
    if(!success){
        Serial.printf("WARNING: overwriting old file: %s \n", filename);
    }
    changeCurrentFilename(filename);
    return success;    
}*/

/*---------------------------------------------------------------------------------*/
uint8_t FileSystem::changeCurrentFilename(const char *filename)
{
    uint8_t success = 0;
    if (strlen(filename) < 20){
        strncpy(_current_filename, filename, 30);
        success = createFile(filename); 
    }
    return success;
}

/*---------------------------------------------------------------------------------*/
uint8_t FileSystem::storeMeasurement(float *power, float *pitch, float *roll, uint8_t *temp)
{
    /* Possible measurements and their max values:
     * --------------
     * power: 1999.9
     * pitch: -90.0
     * roll : -90.0
     * temp : 70
     * --------------
     * together it takes 18 chars for measurements + 3 for comma delimiter (,) + /0
     */

    // Taking one more just in case if some - appearers at temperatures
    char str[23];

    // Store the values into string
    sprintf(str, "%4.1f,%3.1f,%3.1f,%d\n", *power, *pitch, *roll, *temp);

    // Return 0 if there is an error
    return append(_current_filename, str);
}

/*---------------------------------------------------------------------------------*/
uint8_t FileSystem::storeMeasurementWTimstamp(float *power, float *pitch, float *roll, uint8_t *temp, uint8_t *h, uint8_t *m, uint8_t *s)
{
    /* Possible measurements and their max values:
     * --------------
     * power: 1999.9
     * pitch: -90.0
     * roll : -90.0
     * temp : 70
     * --------------
     * together it takes 18 chars for measurements + 3 for comma delimiter (,) + /0
     * plus another
     * --------------
     * time: 081032
     * --------------
     * 6 chars for time + 1 for comma delimiter (,)
     */
    
    // Taking one more just in case if some - appearers at temperatures
    char str[30];

    // Store the values into string
    sprintf(str, "%02d%02d%02d,%3.1f,%3.1f,%3.1f,%d\n", *h, *m, *s, *power, *pitch, *roll, *temp);

    // Return 0 if there is an error
    return append(_current_filename, str);
}






/*---------------------------------------------------------------------------------*/
uint8_t FileSystem::append(const char *path, const char *msg)
{
    File file = SPIFFS.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return 0;
    }
    if(!file.print(msg)){
        Serial.println("- append failed");
        return 0;
    }
    file.close();
    return 1;
}


/*---------------------------------------------------------------------------------*/
/*
//Check for existing files or not?
uint8_t SERVER_check4html(void)
{
    const char *filename;
    uint8_t html = 0, css = 0, js = 0, g = 0;

    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    while(file){
        filename = file.name();

        if (strcmp(filename, INDEX_FILE) == 0){
            html = 1;
        }
        else if (strcmp(filename, CSS_FILE) == 0){
            css = 1;
        }
        else if (strcmp(filename, JS_FILE) == 0) {
            js = 1;
        }
        else if (strcmp(filename, GRAPH_FILE) == 0) {
            g = 1;
        }
        else{
            Serial.println("W: Found additional file:");
            Serial.println(filename);
            // TODO Delete it?
        }
        file = root.openNextFile();
    }

    if ((html && css) && (g && js)) {
        return 1;
    }
    else {
        Serial.printf("Found files: html %d, css %d, js %d, graf %d \n", html, css, js, g);
        return 0;
    }
}
*/
