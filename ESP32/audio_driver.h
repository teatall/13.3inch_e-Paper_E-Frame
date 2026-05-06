#pragma once

// 🎵 音频驱动与资源
#include "sound_action.h"
#include <driver/i2s.h>
#include <Wire.h> 

// 音频引脚定义
#define I2S_BCLK      21
#define I2S_LRCK      47
#define I2S_DOUT      45 
#define I2S_MCLK      14 
#define PA_CTRL_PIN   13 

// 音频全局状态与配置
inline uint8_t ES8311_ADDR = 0x18; 
inline volatile bool isTikPlaying = false;
inline TaskHandle_t AudioTaskHandle = NULL;

// ==================== 底层 DAC 初始化 ====================
inline void writeAudioReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(ES8311_ADDR); 
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

inline bool initAudioDAC() {
    Wire.begin(41, 42); 
    Wire.setClock(100000); 
    Wire.beginTransmission(0x18);
    if (Wire.endTransmission() == 0) { ES8311_ADDR = 0x18; }
    else {
        Wire.beginTransmission(0x19);
        if (Wire.endTransmission() == 0) { ES8311_ADDR = 0x19; }
        else { return false; }
    }
    writeAudioReg(0x00, 0x1F); delay(10); 
    writeAudioReg(0x00, 0x00); delay(10);
    writeAudioReg(0x01, 0x3F); 
    writeAudioReg(0x02, 0x00); 
    writeAudioReg(0x03, 0x10);
    writeAudioReg(0x16, 0x24); 
    writeAudioReg(0x0B, 0x00); 
    writeAudioReg(0x0C, 0x0C); 
    writeAudioReg(0x10, 0x1F); 
    writeAudioReg(0x11, 0x7F); 
    writeAudioReg(0x00, 0x80); delay(20);
    writeAudioReg(0x12, 0x00); 
    writeAudioReg(0x13, 0x10); 
    writeAudioReg(0x31, 0x00);
    writeAudioReg(0x32, 0xBF); 
    return true;
}

// ==================== 异步滴答声循环模块 ====================
inline void tikSoundTask(void *pvParameters) {
    size_t header_offset = 44;
    size_t total_len = sizeof(sound_tik);
    size_t pure_len = total_len > 44 ? total_len - 44 : 0;
    for (size_t i = 12; i < total_len - 8 && i < 100; i++) {
        if (sound_tik[i] == 'd' && sound_tik[i+1] == 'a' && sound_tik[i+2] == 't' && sound_tik[i+3] == 'a') {
            pure_len = sound_tik[i+4] | (sound_tik[i+5] << 8) | (sound_tik[i+6] << 16) | (sound_tik[i+7] << 24);
            header_offset = i + 8;
            break;
        }
    }
    if (header_offset + pure_len > total_len) { pure_len = total_len - header_offset; }
    const unsigned char* pure_data = sound_tik + header_offset;
    while (isTikPlaying) {
        size_t bytes_written = 0;
        i2s_write(I2S_NUM_0, pure_data, pure_len, &bytes_written, portMAX_DELAY);
    }
    vTaskDelete(NULL); 
}

inline void startTikLoop() {
    if (isTikPlaying) return; 
    isTikPlaying = true;
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 22050, .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, .dma_buf_count = 4, .dma_buf_len = 512,
        .use_apll = true, .tx_desc_auto_clear = true
    };
    i2s_pin_config_t pin_config = { .mck_io_num = I2S_MCLK, .bck_io_num = I2S_BCLK, .ws_io_num = I2S_LRCK, .data_out_num = I2S_DOUT, .data_in_num = I2S_PIN_NO_CHANGE };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    if (initAudioDAC()) {
        delay(50); pinMode(PA_CTRL_PIN, OUTPUT);
        digitalWrite(PA_CTRL_PIN, HIGH); delay(20);
        xTaskCreatePinnedToCore(tikSoundTask, "tikTask", 4096, NULL, 1, &AudioTaskHandle, 0);
    }
}

inline void stopTikLoop() {
    isTikPlaying = false;
    if (AudioTaskHandle != NULL) { vTaskDelete(AudioTaskHandle); AudioTaskHandle = NULL; }
    digitalWrite(PA_CTRL_PIN, LOW); i2s_driver_uninstall(I2S_NUM_0);
}

// ==================== 单次音效播放 ====================
inline void playSoundOnce(const unsigned char* audio_data, size_t length) {
    size_t header_offset = 44;
    size_t pure_len = length > 44 ? length - 44 : 0;
    for (size_t i = 12; i < length - 8 && i < 100; i++) {
        if (audio_data[i] == 'd' && audio_data[i+1] == 'a' && audio_data[i+2] == 't' && audio_data[i+3] == 'a') {
            pure_len = audio_data[i+4] | (audio_data[i+5] << 8) | (audio_data[i+6] << 16) | (audio_data[i+7] << 24);
            header_offset = i + 8; break;
        }
    }
    if (header_offset + pure_len > length) { pure_len = length - header_offset; }
    if (pure_len == 0) return;
    const unsigned char* pure_audio_data = audio_data + header_offset;
    i2s_config_t i2s_config = { .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), .sample_rate = 22050, .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, .communication_format = I2S_COMM_FORMAT_STAND_I2S, .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, .dma_buf_count = 4, .dma_buf_len = 512, .use_apll = true, .tx_desc_auto_clear = true };
    i2s_pin_config_t pin_config = { .mck_io_num = I2S_MCLK, .bck_io_num = I2S_BCLK, .ws_io_num = I2S_LRCK, .data_out_num = I2S_DOUT, .data_in_num = I2S_PIN_NO_CHANGE };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    if (initAudioDAC()) {
        delay(50); pinMode(PA_CTRL_PIN, OUTPUT);
        digitalWrite(PA_CTRL_PIN, HIGH); delay(20); 
        size_t bytes_written = 0;
        TickType_t timeout = pdMS_TO_TICKS((pure_len * 1000 / 44100) + 1000);
        i2s_write(I2S_NUM_0, pure_audio_data, pure_len, &bytes_written, timeout);
        delay(200); digitalWrite(PA_CTRL_PIN, LOW);
    }
    i2s_driver_uninstall(I2S_NUM_0);
}