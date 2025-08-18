#ifndef __SDCARD_H__
#define __SDCARD_H__
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "stdio.h"
#include "app.h"
#include "fatfs.h"

class SDCard {
    public:
        SDCard(void);
        colirone_err_t mount(void);
        colirone_err_t unmount(void);
        colirone_err_t createFolder(const char* foldername);
        colirone_err_t browseFiles(const char* foldername);

        colirone_err_t createFile(const char* filePath);
        colirone_err_t openFile(const char* filePath);
        colirone_err_t writeData(const char* filePath, const char* data, size_t size);
        colirone_err_t closeFile(void);

        colirone_err_t appendToFile(const char* filePath, const char* data, size_t size);
        
        colirone_err_t deleteFile(const char* filePath);
        colirone_err_t checkExistsFolder(const char* folderName);
        colirone_err_t checkExistsFile(const char* filePath);
        
        uint32_t getFreeSpace(void);
        uint32_t getTotalSize(void);
        uint32_t getFileSize(const char* filePath);
        uint32_t getFolderSize(const char* foldername);
        bool getMountStatus(void);
    private:
        bool isMounted;
        FATFS FatFs;
        FIL Fil;
        FRESULT FR_Status;
        FATFS *FS_Ptr;
        UINT RWC, WWC; // Read/Write Word Counter
        DWORD FreeClusters;
        uint32_t totalSize, freeSpace;
        FILINFO fno;
        DIR dir;

        const char* errorToString(FRESULT res);
};

#endif