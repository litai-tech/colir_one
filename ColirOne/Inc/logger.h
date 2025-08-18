                 #ifndef __LOGGER_H__
#define __LOGGER_H__
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "stdio.h"
#include "app.h"
#include "rf.h"
#include "sdcard.h"

typedef struct __attribute__((packed)) {
    uint16_t pageSize;
    uint32_t pageCount;
    uint32_t sectorSize;
    uint32_t sectorCount;
    uint32_t blockSize;
    uint32_t blockCount;
    uint32_t capacityInKiloByte;
} colirone_storage_info_t;

class Logger {
    public: 
        Logger(void);
        colirone_err_t init(void);
        colirone_storage_info_t getStorageInfo();
        bool checkEnableWriteLogs(void);
        void storeLog(uint8_t *data, uint32_t size);
        void storeSensorLog(colirone_payload_sensor_t *sensorData);
        void storeCommandLog(colirone_payload_cmd_t *commandData);
        void readAllLogs(void);
        void eraseAllLogs(void);
        uint32_t getUsedSpace(void) const;
        uint32_t getFreeSpace(void) const;
        void readLatestLogs(void);
        void readLatestLogs(uint32_t numPages);
        void startLogging(void);
        void stopLogging(void);

        void getLogFileIndex(uint32_t* logFileIndex);
        void updateLogFileIndex(void);
        void resetLogFileIndex(void);

        colirone_err_t initLogFile(const char* logFolderName, const char* logFileName);
        colirone_err_t initLogFile(const char* logFileName);
        colirone_err_t initLogFile(void);

        colirone_err_t writeAllLogsFile(const char* logFolderName, const char* logFileName);
        colirone_err_t writeAllLogsFile(const char* logFileName);
        colirone_err_t writeAllLogsFile(void);

        colirone_err_t writeLastestLogsFile(const char* logFolderName, const char* logFileName, uint32_t numPages);
        colirone_err_t writeLastestLogsFile(const char* logFileName, uint32_t numPages);
        colirone_err_t writeLastestLogsFile(uint32_t numPages);
        colirone_err_t writeLastestLogsFile(void);

        colirone_err_t deleteLogsFile(const char* logFolderName, const char* logFileName);
        colirone_err_t deleteLogsFile(const char* logFileName);
        colirone_err_t deleteLogsFile(void);

        uint32_t getLogFileSize(const char* logFolderName, const char* logFileName);
        uint32_t getLogFileSize(const char* logFileName);
        uint32_t getLogFileSize(void);

        uint32_t getLogFolderSize(const char* logFolderName);
        uint32_t getLogFolderSize(void);

    private:
        SDCard sdcard;
        uint32_t nextFreeAddress;
        uint32_t startLogAddress;
        bool loggingEnabled;
        bool isLogFileExists;
        colirone_storage_info_t info;
        uint32_t getNextFreeAddress(void) const;
        uint32_t findLastFreeAddress(void);
        char* generateDefaultSystemLog(char* logBuffer, uint32_t timestamp);
        bool isPageEmpty(uint32_t pageNumber);
        colirone_err_t eraseSectorIfNeeded(uint32_t address, uint32_t writeSize, uint32_t offsetInPage);
        void loadConfig(void);
        void writeConfig(void);
        void findStartLogAddress(void);
        void printLog(uint32_t startPage, uint32_t endPage, uint32_t lastOffset);
        
        colirone_err_t writeLogToSDCard(const char* logFilePath, uint32_t startPage, uint32_t endPage, uint32_t lastOffset);
};

#endif