#include "logger.h"
#include "../../Module/Inc/w25qxx.h"

const char* DEFAULT_LOG_FOLDER_NAME         = "DEFAULT_COLIR_ONE_LOGS";
const char* DEFAULT_LOG_FILE_NAME           = "colirone_logs.txt";

#define FLASH_PAGE_SIZE         256
#define START_LOG_PAGE          16
#define START_LOG_ADDRESS       (START_LOG_PAGE * FLASH_PAGE_SIZE)
#define END_LOG_PAGE            (32768 - 1)
#define START_LOG_SECTOR        (START_LOG_PAGE / 16)

#define START_CONFIG_PAGE       0
#define END_CONFIG_PAGE         15
#define CONFIG_SECTOR           0
#define CONFIG_SPACE_SIZE       FLASH_PAGE_SIZE * START_LOG_PAGE

#define NULL_DATA               0xFF
typedef struct {
    uint8_t wrapAround;
    uint32_t logFileIndex;
} colirone_config_t;

static colirone_config_t colironeConfig = {0};

Logger::Logger(void) {
    loggingEnabled = false;
    nextFreeAddress = START_LOG_ADDRESS;
    isLogFileExists = false;
}

void Logger::loadConfig(void){
    size_t configSize = sizeof(colirone_config_t);
    W25qxx_ReadBytes((uint8_t*)&colironeConfig, START_CONFIG_PAGE, configSize);
    while(w25qxx.Lock);
    if(colironeConfig.wrapAround == NULL_DATA) {
        colironeConfig.wrapAround = 0; // Default to no wrap around
        writeConfig();
    }
    if(colironeConfig.logFileIndex == NULL_DATA) {
        colironeConfig.logFileIndex = 0; // Default to first log file
        writeConfig();
    }
}

void Logger::writeConfig(void) {
    size_t configSize = sizeof(colirone_config_t);
    W25qxx_EraseSector(CONFIG_SECTOR);
    while(w25qxx.Lock);
    W25qxx_WritePage((uint8_t*)&colironeConfig, START_CONFIG_PAGE, 0, configSize);
    while(w25qxx.Lock);
}

void Logger::getLogFileIndex(uint32_t* logFileIndex) {
    *logFileIndex = colironeConfig.logFileIndex;
}

void Logger::updateLogFileIndex(void){
    colironeConfig.logFileIndex++;
    // Check for overflow - reset to 0 if maximum value reached
    if(colironeConfig.logFileIndex == UINT32_MAX) {
        colironeConfig.logFileIndex = 0;
    }
    writeConfig();
}

void Logger::resetLogFileIndex(void) {
    colironeConfig.logFileIndex = 0;
    writeConfig();
}

uint32_t Logger::findLastFreeAddress(void) {
    uint8_t buffer[FLASH_PAGE_SIZE];
    uint32_t lastFreeAddress = 0;
    for(uint32_t page = START_LOG_PAGE; page < info.pageCount; page++) {
        W25qxx_ReadPage(buffer, page, 0, FLASH_PAGE_SIZE);
        for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
            if(buffer[i] == NULL_DATA) {
                lastFreeAddress = page * FLASH_PAGE_SIZE + i;
                return lastFreeAddress;
            }
        }
    }
    lastFreeAddress = info.pageCount * FLASH_PAGE_SIZE; // No free space found, return end of memory
    return lastFreeAddress;
}

void Logger::findStartLogAddress(void){
    if(!colironeConfig.wrapAround) {
        startLogAddress = START_LOG_ADDRESS;
    }
    else{
        uint32_t pageContainFreeAddress = nextFreeAddress / FLASH_PAGE_SIZE;
        uint32_t sectorContainFreePage = W25qxx_PageToSector(pageContainFreeAddress);
        startLogAddress = (sectorContainFreePage + 1) * w25qxx.SectorSize; //End of the sector containing the next free address
    }    
}

colirone_err_t Logger::init(void) {
    bool init = W25qxx_Init();
    if(init) {
        info.pageSize = w25qxx.PageSize; //256 bytes
        info.pageCount = w25qxx.PageCount; //32768 pages
        info.sectorSize = w25qxx.SectorSize; //4096 bytes
        info.sectorCount = w25qxx.SectorCount; //2048
        info.blockSize = w25qxx.BlockSize; //65536 bytes
        info.blockCount = w25qxx.BlockCount; //128
        info.capacityInKiloByte = w25qxx.CapacityInKiloByte; //8192 KB
        loadConfig();
        nextFreeAddress = findLastFreeAddress();
        findStartLogAddress();
        return COLIRONE_OK;
    }
    else{
        printf("Error: W25Qxx initialization failed.\n");
        return COLIRONE_ERROR;
    } 
}

colirone_storage_info_t Logger::getStorageInfo() {
    return info;
}

bool Logger::isPageEmpty(uint32_t pageNumber) {
    uint8_t buffer[FLASH_PAGE_SIZE];
    W25qxx_ReadPage(buffer, pageNumber, 0, FLASH_PAGE_SIZE);
    
    for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        if(buffer[i] != NULL_DATA) {
            return false;
        }
    }
    return true;
}

colirone_err_t Logger::eraseSectorIfNeeded(uint32_t address, uint32_t writeSize, uint32_t offsetInPage) {
    uint32_t currentSectorNumber = W25qxx_PageToSector(address / FLASH_PAGE_SIZE);
    uint32_t lastPageInSector = (currentSectorNumber + 1) * w25qxx.SectorSize / FLASH_PAGE_SIZE - 1;
    if(!isPageEmpty(lastPageInSector)){
        uint32_t nextSectorNumber = currentSectorNumber + 1;
        printf("Erasing sector %ld due to non-empty next page %ld.\n", nextSectorNumber, lastPageInSector + 1);
        W25qxx_EraseSector(nextSectorNumber);
        while(w25qxx.Lock);
        startLogAddress = (nextSectorNumber + 1) * w25qxx.SectorSize; //End of the sector containing the next free address
        return COLIRONE_OK;
    }

    return COLIRONE_OK;
}

void Logger::storeLog(uint8_t *data, uint32_t size) {
    if(loggingEnabled == false) return;
    uint32_t currentAddr = nextFreeAddress;
    
    if(currentAddr + size > info.pageCount * FLASH_PAGE_SIZE) {
        currentAddr = START_LOG_ADDRESS;
        nextFreeAddress = START_LOG_ADDRESS;
        colironeConfig.wrapAround = 1;
        writeConfig();
        W25qxx_EraseSector(START_LOG_SECTOR);
        while(w25qxx.Lock);
    }

    uint32_t remain = size;
    uint32_t data_offset = 0;

    while(remain > 0) {
        uint32_t pageAddr = currentAddr / FLASH_PAGE_SIZE;
        uint32_t offsetInPage = currentAddr % FLASH_PAGE_SIZE;
    
        uint32_t writeBytes = FLASH_PAGE_SIZE - offsetInPage;
        if(writeBytes > remain) writeBytes = remain;
        if(colironeConfig.wrapAround){
            eraseSectorIfNeeded(currentAddr, writeBytes, offsetInPage);
        }
        printf("write %ld bytes to page %ld, offset %ld\n", writeBytes, pageAddr, offsetInPage);
        W25qxx_WritePage(&data[data_offset], pageAddr, offsetInPage, writeBytes);
        while(w25qxx.Lock);

        currentAddr += writeBytes;
        data_offset += writeBytes;
        remain -= writeBytes;
    }
    
    nextFreeAddress = currentAddr;
}

void Logger::printLog(uint32_t startPage, uint32_t endPage, uint32_t lastOffset) {
    uint8_t buffer[FLASH_PAGE_SIZE];

    for(uint32_t page = startPage; page <= endPage; page++) {
        W25qxx_ReadPage(buffer, page, 0, FLASH_PAGE_SIZE);
        if(page < endPage) {
            for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
                if(buffer[i] != NULL_DATA) printf("%c", buffer[i]);
            }
        } else {
            for(uint32_t i = 0; i < lastOffset; i++) {
                if(buffer[i] != NULL_DATA) printf("%c", buffer[i]);
            }
        }
    }
}

void Logger::readAllLogs(void) {
    uint32_t lastAddr = nextFreeAddress;
    findStartLogAddress();
    uint32_t startPage = startLogAddress / FLASH_PAGE_SIZE;
    if(colironeConfig.wrapAround) {
        printf("Reading logs from page %ld to page %d (wrap-around mode)\n", startPage, END_LOG_PAGE);
        for(uint32_t page = startPage ; page < info.pageCount; page++) {
            uint8_t buffer[FLASH_PAGE_SIZE];
            W25qxx_ReadPage(buffer, page, 0, FLASH_PAGE_SIZE);
            for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
                if(buffer[i] != NULL_DATA) printf("%c", buffer[i]);
            }
        }
    }
    uint32_t lastPage = lastAddr / FLASH_PAGE_SIZE;
    uint32_t lastOffset = lastAddr % FLASH_PAGE_SIZE;
    printf("Reading logs from page %d to page %ld (last offset: %ld)\n", START_LOG_PAGE, lastPage, lastOffset);
    printLog(START_LOG_PAGE, lastPage, lastOffset);
}

void Logger::readLatestLogs(void) {
    readLatestLogs(4); // Default to 4 pages (1KB)
}

void Logger::readLatestLogs(uint32_t numPages) {
    uint32_t lastAddr = nextFreeAddress;
    uint32_t lastOffset = lastAddr % FLASH_PAGE_SIZE;
    uint32_t lastPage = lastAddr / FLASH_PAGE_SIZE;
    if(!colironeConfig.wrapAround){
        uint32_t startPage = (lastPage <= numPages) ? START_LOG_PAGE : lastPage - numPages + 1;
        printLog(startPage, lastPage, lastOffset);
    }
    else{
        if(lastPage - START_LOG_PAGE < numPages){
            int32_t readRevesePage = numPages - (lastPage - START_LOG_PAGE);
            int32_t startPage = END_LOG_PAGE - readRevesePage + 1;
            printf("Reading logs from page %ld to page %d\n", startPage, END_LOG_PAGE);
            for(uint32_t page = startPage; page <= END_LOG_PAGE; page++) {
                uint8_t buffer[FLASH_PAGE_SIZE];
                W25qxx_ReadPage(buffer, page, 0, FLASH_PAGE_SIZE);
                for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
                    if(buffer[i] != NULL_DATA) printf("%c", buffer[i]);
                }
            }
            if(numPages - readRevesePage > 0){
                printf("Continuing from page %d to page %ld (last offset: %ld)\n", START_LOG_PAGE, lastPage, lastOffset);
                int32_t startPage = START_LOG_PAGE;
                printLog(startPage, lastPage, lastOffset);
            }        
        }
        else{
            uint32_t startPage = (lastPage <= numPages) ? START_LOG_PAGE : lastPage - numPages + 1;
            printf("\nReading logs from page %ld to page %ld (last offset: %ld)\n", startPage, lastPage, lastOffset);
            printLog(startPage, lastPage, lastOffset);
        }
    }
}

void Logger::eraseAllLogs(void) {
    W25qxx_EraseChip();
    printf("Waiting for erase to complete...\r\n");
    while(w25qxx.Lock);
    colironeConfig.wrapAround = 0;
    writeConfig();
    nextFreeAddress = START_LOG_ADDRESS;
    printf("All logs erased.\r\n");
}

uint32_t Logger::getUsedSpace(void) const {
    uint32_t freeSpace = getFreeSpace();
    uint32_t totalSpace = info.pageCount * FLASH_PAGE_SIZE;
    return (totalSpace - freeSpace < 0) ? 0 : (totalSpace - freeSpace);
}

uint32_t Logger::getFreeSpace(void) const {
    if(!colironeConfig.wrapAround) {
        return (info.pageCount * FLASH_PAGE_SIZE - nextFreeAddress < 0) ? 0 : (info.pageCount * FLASH_PAGE_SIZE - nextFreeAddress);
    }
    else{
        return (startLogAddress - nextFreeAddress < 0) ? 0 : (startLogAddress - nextFreeAddress);
    }
}


void Logger::storeSensorLog(colirone_payload_sensor_t *sensorData) {
    char logBuffer[512];
    sprintf(logBuffer, 
            "Acceleration: (%.2f, %.2f, %.2f), Gyroscope: (%.2f, %.2f, %.2f), Orientation: (%.2f, %.2f, %.2f), Quaternion: (%.2f ,%.2f, %.2f, %.2f), Temperature: %.2f, Pressure: %.2f, Altitude: %.2f, GPS: (%.6f, %.6f, %d), Vertical Velocity: %.2f\n",
            sensorData->acceleration.x, sensorData->acceleration.y, sensorData->acceleration.z,
            sensorData->gyroscope.x, sensorData->gyroscope.y, sensorData->gyroscope.z,
            sensorData->orientation.x, sensorData->orientation.y, sensorData->orientation.z,
            sensorData->quaternion.w, sensorData->quaternion.x, sensorData->quaternion.y, sensorData->quaternion.z,
            sensorData->barometer.temperature,
            sensorData->barometer.pressure,
            sensorData->barometer.altitude,
            sensorData->gps.latitude, sensorData->gps.longitude, sensorData->gps.visibleSatellites,
            sensorData->verticalVelocity);
    
    char* logDefault = generateDefaultSystemLog(logBuffer, HAL_GetTick());
    storeLog((uint8_t*)logDefault, strlen(logDefault));
    free(logDefault);
}

void Logger::storeCommandLog(colirone_payload_cmd_t *commandData) {
    char logBuffer[256];
    sprintf(logBuffer, "Lighter Launch Number: %d, Close Shutes: %d, Open Shutes: %d, Start Logs: %d, Write Logs: %d, Reset Altitude: %d, Remove Logs: %d\n",
            commandData->lighterLaunchNumber,
            commandData->closeShutes,
            commandData->openShutes,
            commandData->startLogs,
            commandData->writeLogs,
            commandData->resetAltitude,
            commandData->removeLogs);
    
    char* logDefault = generateDefaultSystemLog(logBuffer, HAL_GetTick());
    storeLog((uint8_t*)logDefault, strlen(logDefault));
    free(logDefault);
}

char* Logger::generateDefaultSystemLog(char* logBuffer, uint32_t timestamp) {
    char* logDefault = (char*)malloc(576 * sizeof(char));
    sprintf(logDefault, "I (%lu): %s ", timestamp, logBuffer);
    return logDefault;
}

bool Logger::checkEnableWriteLogs(void) {
    return loggingEnabled;
}

void Logger::startLogging(void) {
    loggingEnabled = true;
}

void Logger::stopLogging(void) {
    loggingEnabled = false;
}

// Function to write logs to SD card

colirone_err_t Logger::writeLogToSDCard(const char* logFilePath, uint32_t startPage, uint32_t endPage, uint32_t lastOffset) {
    if(!sdcard.getMountStatus()) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    COLIRONE_CHECK_ERROR(sdcard.openFile(logFilePath));
    uint32_t pageSize = FLASH_PAGE_SIZE;
    for(uint32_t page = startPage; page <= endPage; page++) {
        uint8_t buffer[FLASH_PAGE_SIZE];
        W25qxx_ReadPage(buffer, page, 0, FLASH_PAGE_SIZE);
        if(page < endPage) {
            for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
                if(buffer[i] == NULL_DATA) {
                    pageSize = i;
                    break;
                }    
            }
            COLIRONE_CHECK_ERROR(sdcard.appendToFile(logFilePath, (char*)buffer, pageSize));
        } else {
            COLIRONE_CHECK_ERROR(sdcard.appendToFile(logFilePath, (char*)buffer, lastOffset));
        }
    }
    COLIRONE_CHECK_ERROR(sdcard.closeFile());
    return COLIRONE_OK;
}

colirone_err_t Logger::initLogFile(const char* logFolderName, const char* logFileName) {
    COLIRONE_CHECK_ERROR(sdcard.mount());
    if(sdcard.checkExistsFolder(logFolderName) != COLIRONE_OK) {
        printf("Log folder does not exist, creating...\r\n");
        COLIRONE_CHECK_ERROR(sdcard.createFolder(logFolderName));
    }
    char* logFilePath = (char*)malloc(strlen(logFolderName) + strlen(logFileName) + 2);
    sprintf(logFilePath, "%s/%s", logFolderName, logFileName);
    if(sdcard.checkExistsFile(logFilePath) != COLIRONE_OK) {
        printf("Log file does not exist, creating new one...\r\n");
        COLIRONE_CHECK_ERROR(sdcard.createFile(logFilePath));
    }
    free(logFilePath);
    return COLIRONE_OK;
}

colirone_err_t Logger::initLogFile(const char* logFileName) {
    return initLogFile(DEFAULT_LOG_FOLDER_NAME, logFileName);
}

colirone_err_t Logger::initLogFile(void) {
    return initLogFile(DEFAULT_LOG_FOLDER_NAME, DEFAULT_LOG_FILE_NAME);
}

colirone_err_t Logger::writeAllLogsFile(const char* logFolderName, const char* logFileName) {
    char* logFilePath = (char*)malloc(strlen(logFolderName) + strlen(logFileName) + 2);
    sprintf(logFilePath, "%s/%s", logFolderName, logFileName);
    if(!sdcard.getMountStatus()) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    uint32_t lastAddr = nextFreeAddress;
    findStartLogAddress();
    uint32_t startPage = startLogAddress / FLASH_PAGE_SIZE;
    if(colironeConfig.wrapAround) {
        printf("Writing logs from page %ld to page %d (wrap-around mode)\n", startPage, END_LOG_PAGE);
        COLIRONE_CHECK_ERROR(sdcard.openFile(logFilePath));
        uint32_t pageSize = FLASH_PAGE_SIZE;
        for(uint32_t page = startPage ; page < info.pageCount; page++) {
            uint8_t buffer[FLASH_PAGE_SIZE];
            W25qxx_ReadPage(buffer, page, 0, FLASH_PAGE_SIZE);
            for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
                if(buffer[i] == NULL_DATA) {
                    pageSize = i;
                    break;
                }    
            }
            COLIRONE_CHECK_ERROR(sdcard.appendToFile(logFilePath, (char*)buffer, pageSize));
        }
        COLIRONE_CHECK_ERROR(sdcard.closeFile());
    }
    uint32_t lastPage = lastAddr / FLASH_PAGE_SIZE;
    uint32_t lastOffset = lastAddr % FLASH_PAGE_SIZE;
    printf("Writing logs from page %d to page %ld (last offset: %ld)\n", START_LOG_PAGE, lastPage, lastOffset);
    COLIRONE_CHECK_ERROR(writeLogToSDCard(logFilePath, START_LOG_PAGE, lastPage, lastOffset));
    free(logFilePath);
    printf("All logs written to file: %s\r\n", logFileName);
    return COLIRONE_OK;
}

colirone_err_t Logger::writeAllLogsFile(const char* logFileName) {
    return writeAllLogsFile(DEFAULT_LOG_FOLDER_NAME, logFileName);
}

colirone_err_t Logger::writeAllLogsFile(void) {
    return writeAllLogsFile(DEFAULT_LOG_FOLDER_NAME, DEFAULT_LOG_FILE_NAME);
}

colirone_err_t Logger::writeLastestLogsFile(const char* logFolderName, const char* logFileName, uint32_t numPages){
    char* logFilePath = (char*)malloc(strlen(logFolderName) + strlen(logFileName) + 2);
    sprintf(logFilePath, "%s/%s", logFolderName, logFileName);
    if(!sdcard.getMountStatus()) {
        printf("SD Card not mounted!\r\n");
        free(logFilePath);
        return COLIRONE_ERROR;
    }
    uint32_t lastAddr = nextFreeAddress;
    uint32_t lastOffset = lastAddr % FLASH_PAGE_SIZE;
    uint32_t lastPage = lastAddr / FLASH_PAGE_SIZE;
    if(!colironeConfig.wrapAround){
        uint32_t startPage = (lastPage <= numPages) ? START_LOG_PAGE : lastPage - numPages + 1;
        COLIRONE_CHECK_ERROR(writeLogToSDCard(logFilePath, startPage, lastPage, lastOffset));
    }
    else{
        if(lastPage - START_LOG_PAGE < numPages){
            int32_t readRevesePage = numPages - (lastPage - START_LOG_PAGE);
            int32_t startPage = END_LOG_PAGE - readRevesePage + 1;
            COLIRONE_CHECK_ERROR(sdcard.openFile(logFilePath));
            uint32_t pageSize = FLASH_PAGE_SIZE;
            for(uint32_t page = startPage; page <= END_LOG_PAGE; page++) {
                uint8_t buffer[FLASH_PAGE_SIZE];
                W25qxx_ReadPage(buffer, page, 0, FLASH_PAGE_SIZE);
                for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
                    if(buffer[i] == NULL_DATA) {
                        pageSize = i;
                        break;
                    }
                }
                COLIRONE_CHECK_ERROR(sdcard.appendToFile(logFilePath, (char*)buffer, pageSize));
            }
            COLIRONE_CHECK_ERROR(sdcard.closeFile());
            if(numPages - readRevesePage > 0){
                int32_t startPage = START_LOG_PAGE;
                COLIRONE_CHECK_ERROR(writeLogToSDCard(logFilePath, startPage, lastPage, lastOffset));
            }
        }
        else{
            uint32_t startPage = (lastPage <= numPages) ? START_LOG_PAGE : lastPage - numPages + 1;
            COLIRONE_CHECK_ERROR(writeLogToSDCard(logFilePath, startPage, lastPage, lastOffset));
        }
    }
    free(logFilePath);
    return COLIRONE_OK;
}

colirone_err_t Logger::writeLastestLogsFile(const char* logFileName, uint32_t numPages) {
    return writeLastestLogsFile(DEFAULT_LOG_FOLDER_NAME, logFileName, numPages);
}

colirone_err_t Logger::writeLastestLogsFile(uint32_t numPages) {
    return writeLastestLogsFile(DEFAULT_LOG_FOLDER_NAME, DEFAULT_LOG_FILE_NAME, numPages);
}

colirone_err_t Logger::writeLastestLogsFile(void) {
    return writeLastestLogsFile(DEFAULT_LOG_FOLDER_NAME, DEFAULT_LOG_FILE_NAME, 4); // Default to 4 pages (1KB)
}

colirone_err_t Logger::deleteLogsFile(const char* logFolderName, const char* logFileName) {
    char* logFilePath = (char*)malloc(strlen(logFolderName) + strlen(logFileName) + 2);
    sprintf(logFilePath, "%s/%s", logFolderName, logFileName);
    if(!sdcard.getMountStatus()) {
        printf("SD Card not mounted!\r\n");
        return COLIRONE_ERROR;
    }
    colirone_err_t err = sdcard.deleteFile(logFilePath);
    if(err != COLIRONE_OK) {
        printf("Failed to delete log file %s, Error Code: (%i)\r\n", logFilePath, err);
        free(logFilePath);
        return err;
    }
    printf("Log file %s deleted successfully.\r\n", logFilePath);
    free(logFilePath);
    return COLIRONE_OK;
}

colirone_err_t Logger::deleteLogsFile(const char* logFileName) {
    return deleteLogsFile(DEFAULT_LOG_FOLDER_NAME, logFileName);
}

colirone_err_t Logger::deleteLogsFile(void) {
    return deleteLogsFile(DEFAULT_LOG_FOLDER_NAME, DEFAULT_LOG_FILE_NAME);
}

uint32_t Logger::getLogFileSize(const char* logFolderName, const char* logFileName) {
    char* logFilePath = (char*)malloc(strlen(logFolderName) + strlen(logFileName) + 2);
    sprintf(logFilePath, "%s/%s", logFolderName, logFileName);
    if(!sdcard.getMountStatus()) {
        printf("SD Card not mounted!\r\n");
        free(logFilePath);
        return 0;
    }
    uint32_t fileSize = sdcard.getFileSize(logFilePath);
    free(logFilePath);
    return fileSize;
}

uint32_t Logger::getLogFileSize(const char* logFileName) {
    return getLogFileSize(DEFAULT_LOG_FOLDER_NAME, logFileName);
}

uint32_t Logger::getLogFileSize(void) {
    return getLogFileSize(DEFAULT_LOG_FOLDER_NAME, DEFAULT_LOG_FILE_NAME);
}

uint32_t Logger::getLogFolderSize(const char* logFolderName) {
    if(!sdcard.getMountStatus()) {
        printf("SD Card not mounted!\r\n");
        return 0;
    }
    uint32_t folderSize = sdcard.getFolderSize(logFolderName);
    return folderSize;
}

uint32_t Logger::getLogFolderSize(void) {
    return getLogFolderSize(DEFAULT_LOG_FOLDER_NAME);
}  