#include "logger.h"
#include "../../Module/Inc/w25qxx.h"

#define FLASH_PAGE_SIZE 256

Logger::Logger(void) : nextFreeAddress(0) {
}

colirone_err_t Logger::init(void) {
    bool init = W25qxx_Init();
    if(init) {
        nextFreeAddress = findLastFreeAddress();
        return COLIRONE_OK;
    }
    else return COLIRONE_ERROR;
}

colirone_storage_info_t Logger::getStorageInfo() {
    colirone_storage_info_t info;
    info.pageSize = w25qxx.PageSize;
    info.pageCount = w25qxx.PageCount;
    info.sectorSize = w25qxx.SectorSize;
    info.sectorCount = w25qxx.SectorCount;
    info.blockSize = w25qxx.BlockSize;
    info.blockCount = w25qxx.BlockCount;
    info.capacityInKiloByte = w25qxx.CapacityInKiloByte;
    return info;
}

uint32_t Logger::findLastFreeAddress(void) {
    uint8_t buffer[FLASH_PAGE_SIZE];
    for(uint32_t page = 0; page < w25qxx.PageCount; page++) {
        W25qxx_ReadPage(buffer, page, 0, FLASH_PAGE_SIZE);
        for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
            if(buffer[i] == 0xFF) {
                return page * FLASH_PAGE_SIZE + i;
            }
        }
    }
    return w25qxx.PageCount * FLASH_PAGE_SIZE;
}

bool Logger::isPageEmpty(uint32_t pageNumber) {
    uint8_t buffer[FLASH_PAGE_SIZE];
    W25qxx_ReadPage(buffer, pageNumber, 0, FLASH_PAGE_SIZE);
    
    for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        if(buffer[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

colirone_err_t Logger::eraseSectorIfNeeded(uint32_t address) {
    uint32_t pageNumber = address / FLASH_PAGE_SIZE;
    uint32_t sectorNumber = W25qxx_PageToSector(pageNumber);
    
    if(!isPageEmpty(pageNumber)) {
        W25qxx_EraseSector(sectorNumber);
        while(w25qxx.Lock);
        return COLIRONE_OK;
    }
    return COLIRONE_OK;
}

void Logger::storeLog(uint8_t *data, uint32_t size) {
    uint32_t totalCapacity = w25qxx.PageCount * FLASH_PAGE_SIZE;
    uint32_t currentAddr = nextFreeAddress;
    
    if(currentAddr + size > totalCapacity) {
        currentAddr = 0;
        nextFreeAddress = 0;
    }

    uint32_t remain = size;
    uint32_t data_offset = 0;

    while(remain > 0) {
        uint32_t pageAddr = currentAddr / FLASH_PAGE_SIZE;
        uint32_t offsetInPage = currentAddr % FLASH_PAGE_SIZE;
        
        if(offsetInPage == 0) {
            eraseSectorIfNeeded(currentAddr);
        }
        
        uint32_t writeBytes = FLASH_PAGE_SIZE - offsetInPage;
        if(writeBytes > remain) writeBytes = remain;

        W25qxx_WritePage(&data[data_offset], pageAddr, offsetInPage, writeBytes);
        while(w25qxx.Lock);

        currentAddr += writeBytes;
        data_offset += writeBytes;
        remain -= writeBytes;
    }
    
    // Cập nhật địa chỉ trống tiếp theo
    nextFreeAddress = currentAddr;
}

void Logger::readLogs(void) {
    uint32_t last_addr = nextFreeAddress;
    if(last_addr == 0) return;

    uint32_t last_page = last_addr / FLASH_PAGE_SIZE;
    uint32_t last_offset = last_addr % FLASH_PAGE_SIZE;
    uint8_t buffer[FLASH_PAGE_SIZE];

    for(uint32_t page = 0; page <= last_page; page++) {
        W25qxx_ReadPage(buffer, page, 0, FLASH_PAGE_SIZE);
        if(page < last_page) {
            for(uint32_t i = 0; i < FLASH_PAGE_SIZE; i++) {
                if(buffer[i] != 0xFF) printf("%c", buffer[i]);
            }
        } else {
            for(uint32_t i = 0; i < last_offset; i++) {
                if(buffer[i] != 0xFF) printf("%c", buffer[i]);
            }
        }
    }
}
void Logger::eraseAllLogs(void) {
    W25qxx_EraseChip();
    printf("Waiting for erase to complete...\r\n");
    while(w25qxx.Lock);
    nextFreeAddress = 0;
    printf("All logs erased.\r\n");
}

uint32_t Logger::getNextFreeAddress(void) const {
    return nextFreeAddress;
}

uint32_t Logger::getUsedSpace(void) const {
    return nextFreeAddress;
}

uint32_t Logger::getFreeSpace(void) const {
    uint32_t totalCapacity = w25qxx.PageCount * FLASH_PAGE_SIZE;
    return totalCapacity - nextFreeAddress;
}

bool Logger::hasEnoughSpace(uint32_t dataSize) const {
    uint32_t totalCapacity = w25qxx.PageCount * FLASH_PAGE_SIZE;
    return dataSize <= totalCapacity;
}


void Logger::storeSensorLog(colirone_payload_sensor_t *sensorData) {
    char logBuffer[512];
    sprintf(logBuffer, 
            "Acceleration: (%.2f, %.2f, %.2f), Gyroscope: (%.2f, %.2f, %.2f), Orientation: (%.2f, %.2f, %.2f), Quaternion: (%.2f, %.2f, %.2f), Temperature: %.2f, Pressure: %.2f, Altitude: %.2f, GPS: (%.6f, %.6f, %d), Vertical Velocity: %.2f",
            sensorData->acceleration.x, sensorData->acceleration.y, sensorData->acceleration.z,
            sensorData->gyroscope.x, sensorData->gyroscope.y, sensorData->gyroscope.z,
            sensorData->orientation.x, sensorData->orientation.y, sensorData->orientation.z,
            sensorData->barometer.temperature,
            sensorData->barometer.pressure,
            sensorData->barometer.altitude,
            sensorData->gps.latitude, sensorData->gps.longitude, sensorData->gps.visible_satellites,
            sensorData->vertical_velocity);
    
    char* logDefault = generateDefaultSystemLog(logBuffer, HAL_GetTick());
    storeLog((uint8_t*)logDefault, strlen(logDefault));
    free(logDefault);
}

void Logger::storeCommandLog(colirone_payload_cmd_t *commandData) {
    char logBuffer[256];
    sprintf(logBuffer, "Lighter Launch Number: %d, Close Shutes: %d, Open Shutes: %d, Start Logs: %d, Write Logs: %d, Reset Altitude: %d, Remove Logs: %d",
            commandData->lighter_launch_number,
            commandData->close_shutes,
            commandData->open_shutes,
            commandData->start_logs,
            commandData->write_logs,
            commandData->reset_altitude,
            commandData->remove_logs);
    
    char* logDefault = generateDefaultSystemLog(logBuffer, HAL_GetTick());
    storeLog((uint8_t*)logDefault, strlen(logDefault));
    free(logDefault);
}

char* Logger::generateDefaultSystemLog(char* logBuffer, uint32_t timestamp) {
    char* logDefault = (char*)malloc(576 * sizeof(char));
    sprintf(logDefault, "I (%u): %s ", timestamp, logBuffer);
    return logDefault;
}

bool Logger::checkEnableWriteLogs(void) {
    if(w25qxx.Lock == 0) {
        return true; // Write logs is enabled
    } else {
        return false; // Write logs is disabled
    }
}