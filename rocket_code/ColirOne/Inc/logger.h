
#ifndef LOGGER_H
#define LOGGER_H
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "stdio.h"
#include "app.h"
#include "rf.h"

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
        void readLogs(void);
        void eraseAllLogs(void);
        uint32_t getNextFreeAddress(void) const;
        uint32_t getUsedSpace(void) const;
        uint32_t getFreeSpace(void) const;
        bool hasEnoughSpace(uint32_t dataSize) const;
    private:
        uint32_t nextFreeAddress;
        uint32_t findLastFreeAddress(void);
        char* generateDefaultSystemLog(char* logBuffer, uint32_t timestamp);
        bool isPageEmpty(uint32_t pageNumber);
        colirone_err_t eraseSectorIfNeeded(uint32_t address);
};

#endif