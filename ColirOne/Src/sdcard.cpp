#include "sdcard.h"

SDCard::SDCard(void) {
    isMounted = false;
    totalSize = 0;
    freeSpace = 0;
}

colirone_err_t SDCard::mount(void) {
    FR_Status = f_mount(&FatFs, SDPath, 1);
    if (FR_Status != FR_OK)
    {
      printf("Error! While Mounting SD Card, %s\r\n", errorToString(FR_Status));
      return COLIRONE_ERROR;
    }
    isMounted = true;
    return COLIRONE_OK;
}

colirone_err_t SDCard::unmount(void) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    FR_Status = f_mount(NULL, SDPath, 0);
    if (FR_Status != FR_OK) {
        printf("Error! While Unmounting SD Card, %s\r\n", errorToString(FR_Status));
        return COLIRONE_ERROR;
    }
    isMounted = false;
    return COLIRONE_OK;
}

colirone_err_t SDCard::createFile(const char* filePath) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    FR_Status = f_open(&Fil, filePath, FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_Status != FR_OK) {
        printf("Error creating file: %s, %s\r\n", filePath, errorToString(FR_Status));
        return COLIRONE_ERROR;
    }
    FR_Status = f_write(&Fil, "", 0, &WWC); // Create an empty file
    f_close(&Fil); // Close the file after creation
    return COLIRONE_OK;
}

colirone_err_t SDCard::openFile(const char* filePath) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    BYTE mode = FA_CREATE_ALWAYS | FA_WRITE | FA_READ;
    if(checkExistsFile(filePath) == COLIRONE_OK) {
        mode = FA_OPEN_EXISTING | FA_WRITE;
    }
    FR_Status = f_open(&Fil, filePath, mode);
    if (FR_Status != FR_OK) {
        printf("Error opening file: %s, %s\r\n", filePath, errorToString(FR_Status));
        return COLIRONE_ERROR;
    }
    return COLIRONE_OK;
}

colirone_err_t SDCard::writeData(const char* filePath, const char* data, size_t size) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    FR_Status = f_write(&Fil, data, size, &WWC);
    if (FR_Status != FR_OK) {
        printf("Error writing to file: %s, %s\r\n", filePath, errorToString(FR_Status));
        f_close(&Fil);
        return COLIRONE_ERROR;
    }
    return COLIRONE_OK;
}

colirone_err_t SDCard::closeFile(void) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    FR_Status = f_close(&Fil);
    if (FR_Status != FR_OK) {
        return COLIRONE_ERROR;
    }
    return COLIRONE_OK;
}

colirone_err_t SDCard::appendToFile(const char* filePath, const char* data, size_t size) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    FR_Status = f_lseek(&Fil, f_size(&Fil)); // Move The File Pointer To The EOF (End-Of-File)
    if(FR_Status != FR_OK)
    {
        printf("Error seeking to end of file: %s, %s\r\n", filePath, errorToString(FR_Status));
        return COLIRONE_ERROR;
    }
    // Write New Line of Text Data To The File
    FR_Status = f_write(&Fil, data, size, &WWC);
    return (FR_Status == FR_OK) ? COLIRONE_OK : COLIRONE_ERROR;
}

colirone_err_t SDCard::deleteFile(const char* filePath) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }

    FR_Status = f_unlink(filePath);
    return (FR_Status == FR_OK) ? COLIRONE_OK : COLIRONE_ERROR;
}

uint32_t SDCard::getFreeSpace(void) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return 0;
    }
    f_getfree("", &FreeClusters, &FS_Ptr);
    freeSpace = (uint32_t)(FreeClusters * FS_Ptr->csize * 0.5);
    return freeSpace;
}

uint32_t SDCard::getTotalSize(void) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return 0;
    }
    f_getfree("", &FreeClusters, &FS_Ptr);
    totalSize = (uint32_t)((FS_Ptr->n_fatent - 2) * FS_Ptr->csize * 0.5);
    return totalSize;
}

bool SDCard::getMountStatus(void) {
    return isMounted;
}

colirone_err_t SDCard::checkExistsFile(const char* filePath) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    FRESULT res = f_stat(filePath, &fno);
    if (res == FR_OK) {
        return COLIRONE_OK;
    }
    return COLIRONE_ERROR;
}

colirone_err_t SDCard::checkExistsFolder(const char* folderName) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    FRESULT res = f_stat(folderName, &fno);
    if (res == FR_OK && (fno.fattrib & AM_DIR)) {
        return COLIRONE_OK;
    }
    return COLIRONE_ERROR;
}

colirone_err_t SDCard::createFolder(const char* foldername) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    FR_Status = f_mkdir(foldername);
    if (FR_Status != FR_OK && FR_Status != FR_EXIST) {
        printf("Error creating folder: %s, %s\r\n", foldername, errorToString(FR_Status));
        return COLIRONE_ERROR;
    }
    return COLIRONE_OK;
}

colirone_err_t SDCard::browseFiles(const char* foldername) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    FR_Status = f_opendir(&dir, foldername);
    if (FR_Status != FR_OK) {
        printf("Error opening directory: %s, %s\r\n", foldername, errorToString(FR_Status));
        return COLIRONE_ERROR;
    }

    while (true) {
        FR_Status = f_readdir(&dir, &fno);
        if (FR_Status != FR_OK || fno.fname[0] == 0) break; // Break on error or end of directory
        printf("File: %s, Size: %lu bytes\n", fno.fname, fno.fsize);
    }
    f_closedir(&dir);
    return COLIRONE_OK;
}

const char* SDCard::errorToString(FRESULT res) {
    switch (res) {
        case FR_OK: return "Succeeded";
        case FR_DISK_ERR: return "A hard error occurred in the low level disk I/O layer";
        case FR_INT_ERR: return "Assertion failed";
        case FR_NOT_READY: return "The physical drive cannot work";
        case FR_NO_FILE: return "Could not find the file";
        case FR_NO_PATH: return "Could not find the path";
        case FR_INVALID_NAME: return "The path name format is invalid";
        case FR_DENIED: return "Access denied due to prohibited access or directory full";
        case FR_EXIST: return "Already exists, access denied due to prohibited access";
        case FR_INVALID_OBJECT: return "The file/directory object is invalid";
        case FR_WRITE_PROTECTED: return "The physical drive is write protected ";
        case FR_INVALID_DRIVE: return "The logical drive number is invalid";
        case FR_NOT_ENABLED: return "The volume has no work area";
        case FR_NO_FILESYSTEM: return "There is no valid FAT volume";
        case FR_MKFS_ABORTED: return "The f_mkfs() aborted due to any problem";
        case FR_TIMEOUT: return "Could not get a grant to access the volume within defined period";
        case FR_LOCKED: return "The operation is rejected according to the file sharing policy";
        case FR_NOT_ENOUGH_CORE: return "LFN working buffer could not be allocated";
        case FR_TOO_MANY_OPEN_FILES: return "Number of open files > _FS_LOCK";
        case FR_INVALID_PARAMETER: return "Given parameter is invalid";
        default: return "Unknown error";
    }
}

uint32_t SDCard::getFileSize(const char* filePath) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return 0;
    }
    if (checkExistsFile(filePath) != COLIRONE_OK) {
        printf("File does not exist: %s\r\n", filePath);
        return 0;
    }
    FR_Status = f_stat(filePath, &fno);
    if (FR_Status != FR_OK) {
        printf("Error getting file size: %s, %s\r\n", filePath, errorToString(FR_Status));
        return 0;
    }
    return fno.fsize;
}

uint32_t SDCard::getFolderSize(const char* foldername) {
    if (!isMounted) {
        printf("SD Card not mounted!\r\n");
        return 0;
    }
    uint32_t totalSize = 0;
    if (checkExistsFolder(foldername) != COLIRONE_OK) {
        printf("Folder does not exist: %s\r\n", foldername);
        return 0;
    }
    FR_Status = f_opendir(&dir, foldername);
    if (FR_Status != FR_OK) {
        printf("Error opening directory: %s, %s\r\n", foldername, errorToString(FR_Status));
        return 0;
    }

    while (true) {
        FR_Status = f_readdir(&dir, &fno);
        if (FR_Status != FR_OK || fno.fname[0] == 0) break; // Break on error or end of directory
        if (!(fno.fattrib & AM_DIR)) { // Only count files, not directories
            totalSize += fno.fsize;
        }
    }
    f_closedir(&dir);
    return totalSize;
}

