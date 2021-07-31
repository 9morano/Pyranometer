
#include "pyrano_fs.h"


uint8_t FileSystem::setup(void)
{
    if(!SPIFFS.begin()){
        Serial.println("Error while mounting SPIFFS");
        return 0;
    }
    return 1;
}

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


void FileSystem::printMeasurementsFiles(void)
{
    File measurements = SPIFFS.open("/measure");
    
    Serial.println("Measurements found: ");
    while(true){
        File file = measurements.openNextFile();
        
        if(!file){
            Serial.println("None!");
            break;
        }

        Serial.print(file.name());
        Serial.print(" --> size:");
        Serial.println(file.size(), DEC);
        file.close();
    }
}

void FileSystem::createDir(void)
{
    if (!SPIFFS.mkdir(_measurement_dir)){
        Serial.println("mkdir failed");
    }
}

void FileSystem::createFile(const char *path)
{
    File f = SPIFFS.open("measure/"+path, FILE_WRITE);
    if(!f){
        Serial.printf("Failed to create measurements %s file \n", path);
    }
}

void FileSystem::deleteFile(const char *path)
{
    Serial.printf("Deleting file: %s\r\n", path);
    if(!SPIFFS.remove(path)){
        Serial.println("- delete failed");
    }
}


void FileSystem::append(const char *path, const char *msg)
{
    File file = SPIFFS.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(!file.print(msg)){
        Serial.println("- append failed");
    }
    file.close();
}











/*
// TODO - do we need to check for existing files or not?
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
