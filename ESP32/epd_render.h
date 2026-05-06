#pragma once
#include <Arduino.h>
#include "FS.h"
#include "EPD_13in3e.h"

// 引用主文件中的全局尺寸变量
extern uint16_t SYS_WIDTH;
extern uint16_t SYS_HEIGHT;

// E6 色板映射矩阵
inline const int E6_COLOR_MAP[6][4] = {
    {0x00, 0x00, 0x00, EPD_13IN3E_BLACK},
    {0xFF, 0xFF, 0xFF, EPD_13IN3E_WHITE},
    {0xFF, 0xFF, 0x00, EPD_13IN3E_YELLOW},
    {0xFF, 0x00, 0x00, EPD_13IN3E_RED},
    {0x00, 0x00, 0xFF, EPD_13IN3E_BLUE},
    {0x00, 0xFF, 0x00, EPD_13IN3E_GREEN}
};

// 色彩空间匹配算法
inline uint8_t getNearestE6ColorRGB(uint8_t r, uint8_t g, uint8_t b) {
    int minDistance = 999999;
    uint8_t bestColorCmd = EPD_13IN3E_WHITE;
    for (int i = 0; i < 6; i++) {
        int dr = r - E6_COLOR_MAP[i][0]; 
        int dg = g - E6_COLOR_MAP[i][1]; 
        int db = b - E6_COLOR_MAP[i][2];
        if ((dr*dr + dg*dg + db*db) < minDistance) {
            minDistance = (dr*dr + dg*dg + db*db);
            bestColorCmd = E6_COLOR_MAP[i][3]; 
        }
    }
    return bestColorCmd;
}

// BMP 字节解析辅助函数
inline uint32_t read32(File& f) { 
    uint8_t b[4]; f.read(b, 4); 
    return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24); 
}

inline uint16_t read16(File& f) { 
    uint8_t b[2]; f.read(b, 2); 
    return b[0] | (b[1] << 8); 
}

// 内存图像居中渲染算法
inline void renderMemoryBMPCentered(const unsigned char* bmp_data, uint8_t* buffer) {
    uint32_t dataOffset = bmp_data[10] | (bmp_data[11] << 8) | (bmp_data[12] << 16) | (bmp_data[13] << 24);
    int32_t img_w = bmp_data[18] | (bmp_data[19] << 8) | (bmp_data[20] << 16) | (bmp_data[21] << 24);
    int32_t img_h_raw = bmp_data[22] | (bmp_data[23] << 8) | (bmp_data[24] << 16) | (bmp_data[25] << 24);
    bool bottomUp = true; 
    int32_t img_h = img_h_raw;
    if (img_h_raw < 0) { img_h = -img_h_raw; bottomUp = false; }
    
    int bytesPerPixel = 3; 
    int padding = (4 - ((img_w * bytesPerPixel) % 4)) % 4;
    int start_x = (SYS_WIDTH - img_w) / 2; 
    int start_y = (SYS_HEIGHT - img_h) / 2;
    uint32_t cursor = dataOffset;
    
    for (int row = 0; row < img_h; row++) {
        int source_y = bottomUp ? (img_h - 1 - row) : row; 
        int target_y = start_y + source_y;
        for (int col = 0; col < img_w; col++) {
            int target_x = start_x + col;
            uint8_t b = bmp_data[cursor++]; 
            uint8_t g = bmp_data[cursor++]; 
            uint8_t r = bmp_data[cursor++];
            if (target_x >= 0 && target_x < SYS_WIDTH && target_y >= 0 && target_y < SYS_HEIGHT) {
                uint8_t colorCmd = getNearestE6ColorRGB(r, g, b);
                uint32_t addr = target_y * (SYS_WIDTH / 2) + (target_x / 2);
                if (target_x % 2 == 0) buffer[addr] = (buffer[addr] & 0x0F) | (colorCmd << 4);
                else buffer[addr] = (buffer[addr] & 0xF0) | colorCmd;
            }
        }
        cursor += padding;
    }
}