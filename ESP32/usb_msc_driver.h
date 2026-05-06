#pragma once
#include <Arduino.h>
#include "SD.h"

// 将 SD 卡底层扇区数据桥接给 TinyUSB 的回调函数
inline int32_t msc_onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    uint32_t count = bufsize / 512;
    for (uint32_t i = 0; i < count; i++) { 
        SD.readRAW((uint8_t*)buffer + (i * 512), lba + i); 
    }
    return bufsize;
}

inline int32_t msc_onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    uint32_t count = bufsize / 512;
    for (uint32_t i = 0; i < count; i++) { 
        SD.writeRAW(buffer + (i * 512), lba + i); 
    }
    return bufsize;
}

inline bool msc_onStartStop(uint8_t power_condition, bool start, bool load_eject) { 
    return true; 
}