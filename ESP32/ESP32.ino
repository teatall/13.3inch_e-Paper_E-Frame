#include "EPD_13in3e.h"
#include "GUI_Paint.h"
#include "fonts.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <vector>
#include <algorithm>
// 引入时间相关的头文件
#include <sys/time.h>
#include <Preferences.h>
Preferences prefs;

#include "images.h"
#include <WiFi.h>
#include <WebServer.h>
// 🚀 引入分离后的前端体系
#include "webpage.h"
#include "static_assets.h"
#include "audio_driver.h"
#include "epd_render.h"
#include "usb_msc_driver.h"
#include "USB.h"
#include "USBMSC.h"

USBMSC MSC;
extern "C" bool tud_mounted(void);
extern "C" bool tud_connected(void);

WebServer server(80);

// ==================== 系统配置与版本 ====================
const char* FIRMWARE_VERSION = "v1.1.5";
#define VOLTAGE_DIVIDER 3.263

int image_index = 0;
int sleep_interval_index = 2;
const uint32_t INTERVAL_MAP[8] = { 600, 1800, 3600, 7200, 10800, 21600, 43200, 86400 };

bool web_exit_request = false;

SPIClass sdSPI(HSPI);
#define SD_MISO 5
#define SD_MOSI 7
#define SD_SCK 6
#define SD_CS 15
#define BOOT_BTN 0
#define BAT_ADC_PIN 8
#define LOW_BAT_THRESHOLD 10.0
#define CHARGE_STATE_PIN 38

uint16_t SYS_WIDTH = 0;
uint16_t SYS_HEIGHT = 0;
uint8_t* EPD_Buffer = nullptr;
bool need_rotate = false;

struct FileRecord {
  String path;
  time_t modTime;
};

float getBatteryPercentage() {
    // 充电中不计算百分比
    pinMode(CHARGE_STATE_PIN, INPUT_PULLUP);
    if (digitalRead(CHARGE_STATE_PIN) == LOW) {
        return 101.0f; // 调用方判断 >100 表示充电中
    }
    
    // 多次采样取均值
    uint32_t sum = 0;
    const int SAMPLES = 16;
    for (int i = 0; i < SAMPLES; i++) {
        sum += analogReadMilliVolts(BAT_ADC_PIN);
        delay(5);
    }
    float pin_voltage = (float)(sum / SAMPLES) / 1000.0f;
    float bat_voltage = pin_voltage * VOLTAGE_DIVIDER;
    Serial.printf("[BAT] pin=%.3fV  bat=%.3fV\n", pin_voltage, bat_voltage);
    
    float percentage;
    if      (bat_voltage >= 4.20f) percentage = 100.0f;
    else if (bat_voltage >= 4.00f) percentage = 80.0f + ((bat_voltage - 4.00f) / 0.20f) * 20.0f;
    else if (bat_voltage >= 3.80f) percentage = 50.0f + ((bat_voltage - 3.80f) / 0.20f) * 30.0f;
    else if (bat_voltage >= 3.60f) percentage = 20.0f + ((bat_voltage - 3.60f) / 0.20f) * 30.0f;
    else if (bat_voltage >= 3.30f) percentage =          ((bat_voltage - 3.30f) / 0.30f) * 20.0f;
    else                           percentage = 0.0f;
    
    return constrain(percentage, 0.0f, 100.0f);
}

void showStatusIconAndSleep(const unsigned char* icon_bmp) {
  uint32_t bufferSize = (SYS_WIDTH / 2) * SYS_HEIGHT;
  EPD_Buffer = (uint8_t*)ps_malloc(bufferSize);
  if (EPD_Buffer != NULL) {
    memset(EPD_Buffer, 0x11, bufferSize);
    renderMemoryBMPCentered(icon_bmp, EPD_Buffer);
    DEV_Module_Init();
    delay(200);
    EPD_13IN3E_Init();
    EPD_13IN3E_Display(EPD_Buffer);
    EPD_13IN3E_Sleep();
    DEV_Module_Exit();
    free(EPD_Buffer);
  }
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BOOT_BTN, 0);
  esp_deep_sleep_start();
}

void clearScreenAndShutdown() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  delay(1000);

  uint32_t bufferSize = (SYS_WIDTH / 2) * SYS_HEIGHT;
  EPD_Buffer = (uint8_t*)ps_malloc(bufferSize);
  if (EPD_Buffer != NULL) {
    memset(EPD_Buffer, 0x11, bufferSize);
    DEV_Module_Init();
    delay(200);
    EPD_13IN3E_Init();
    EPD_13IN3E_Display(EPD_Buffer);
    EPD_13IN3E_Sleep();
    DEV_Module_Exit();
    free(EPD_Buffer);
  }
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BOOT_BTN, 0);
  esp_deep_sleep_start();
}

void checkManagementMode() {
  web_exit_request = false;
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (SD.begin(SD_CS, sdSPI)) {
    if (!SD.exists("/.thumbnails")) { SD.mkdir("/.thumbnails"); }
    MSC.vendorID("ESP32");
    MSC.productID("USB_MSC");
    MSC.productRevision("1.0");
    MSC.onRead(msc_onRead);
    MSC.onWrite(msc_onWrite);
    MSC.onStartStop(msc_onStartStop);
    MSC.mediaPresent(true);
    MSC.begin(SD.numSectors(), SD.sectorSize());
  }
  
  USB.begin();
  bool is_usb_connected = false;
  for (int i = 0; i < 100; i++) {
    if (tud_connected()) {
      is_usb_connected = true;
      break;
    }
    delay(10);
  }
  
  if (is_usb_connected) {
    Serial.println("[System] USB Host detected. Entering U-Disk Mode...");
    uint32_t bufferSize = (SYS_WIDTH / 2) * SYS_HEIGHT;
    EPD_Buffer = (uint8_t*)ps_malloc(bufferSize);
    if (EPD_Buffer != NULL) {
      memset(EPD_Buffer, 0x11, bufferSize);
      renderMemoryBMPCentered(usb_drive_icon_bmp, EPD_Buffer);
      DEV_Module_Init();
      delay(200);
      EPD_13IN3E_Init();
      EPD_13IN3E_Display(EPD_Buffer);
      EPD_13IN3E_Sleep();
      DEV_Module_Exit();
      free(EPD_Buffer);
    }
    while (1) {
      if (digitalRead(BOOT_BTN) == LOW) {
        delay(50);
        if (digitalRead(BOOT_BTN) == LOW) {
          while (digitalRead(BOOT_BTN) == LOW) { delay(10); }
          ESP.restart();
        }
      }
      delay(10);
    }
  } else {
    MSC.mediaPresent(false);
    String ssid = "";
    String pass = "";
    bool has_config = false;
    
    // 1. 优先检查 SD 卡是否有新的 wifi.txt (用于首次配置或切换网络)
    if (SD.exists("/wifi.txt")) {
      File wifiFile = SD.open("/wifi.txt");
      if (wifiFile) {
        if (wifiFile.available()) {
          ssid = wifiFile.readStringUntil('\n');
          ssid.trim();
        }
        if (wifiFile.available()) {
          pass = wifiFile.readStringUntil('\n');
          pass.trim();
        }
        wifiFile.close();
        
        if (ssid.length() > 0) {
          prefs.putString("wifi_ssid", ssid);
          prefs.putString("wifi_pass", pass);
          SD.remove("/wifi.txt");
          has_config = true;
          Serial.println("[System] New WiFi config loaded from SD and saved to NVS. wifi.txt deleted.");
        }
      }
    }
    
    // 2. 如果 SD 卡没有配置文件，尝试从 NVS 读取历史配置
    if (!has_config) {
      ssid = prefs.getString("wifi_ssid", "");
      pass = prefs.getString("wifi_pass", "");
      if (ssid.length() > 0) {
        has_config = true;
        Serial.println("[System] WiFi config loaded from internal NVS.");
      }
    }
    
    const unsigned char* target_icon = NULL;
    bool wifi_connected = false;
    
    if (!has_config) {
      target_icon = wifi_no_config_icon_bmp;
    } else {
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(), pass.c_str());
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
      }
      if (WiFi.status() == WL_CONNECTED) {
        target_icon = wifi_on_icon_bmp;
        wifi_connected = true;
      } else {
        target_icon = wifi_off_icon_bmp;
      }
    }
    
    uint32_t bufferSize = (SYS_WIDTH / 2) * SYS_HEIGHT;
    EPD_Buffer = (uint8_t*)ps_malloc(bufferSize);
    if (EPD_Buffer != NULL) {
      memset(EPD_Buffer, 0x11, bufferSize);
      if (target_icon != NULL) renderMemoryBMPCentered(target_icon, EPD_Buffer);
      if (wifi_connected) {
        Paint_NewImage(EPD_Buffer, SYS_WIDTH, SYS_HEIGHT, 0, EPD_13IN3E_WHITE);
        Paint_SetScale(6);
        String ip_str = "http://" + WiFi.localIP().toString();
        Paint_DrawString_EN((SYS_WIDTH - (ip_str.length() * 17)) / 2, (SYS_HEIGHT / 2) + 150, ip_str.c_str(), &Font24, EPD_13IN3E_WHITE, EPD_13IN3E_BLACK);
      }
      DEV_Module_Init();
      delay(200);
      EPD_13IN3E_Init();
      EPD_13IN3E_Display(EPD_Buffer);
      EPD_13IN3E_Sleep();
      DEV_Module_Exit();
      free(EPD_Buffer);
    }
    
    if (wifi_connected) {
      server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", index_html);
      });
      server.on("/styles.css", HTTP_GET, []() {
        server.send(200, "text/css", styles_css);
      });
      server.on("/cropper.min.css", HTTP_GET, []() {
        server.send(200, "text/css", cropper_css);
      });
      server.on("/cropper.min.js", HTTP_GET, []() {
        server.send(200, "application/javascript", cropper_js);
      });
      
      server.on("/api/time", HTTP_POST, []() {
        if (server.hasArg("y") && server.hasArg("m") && server.hasArg("d") && server.hasArg("h") && server.hasArg("min") && server.hasArg("s")) {
          struct tm t;
          t.tm_year = server.arg("y").toInt() - 1900;
          t.tm_mon = server.arg("m").toInt() - 1;
          t.tm_mday = server.arg("d").toInt();
          t.tm_hour = server.arg("h").toInt();
          t.tm_min = server.arg("min").toInt();
          t.tm_sec = server.arg("s").toInt();
          t.tm_isdst = -1;
          time_t timeSinceEpoch = mktime(&t);
          struct timeval now = { .tv_sec = timeSinceEpoch, .tv_usec = 0 };
          settimeofday(&now, NULL);
          Serial.println("[System] Time synced from frontend.");
        }
        server.send(200, "text/plain", "Time synced");
      });
      
      server.on("/api/exit", HTTP_POST, []() {
        server.send(200, "application/json", "{\"status\":\"ok\"}");
        web_exit_request = true;
      });
      
      server.on("/api/img", HTTP_GET, []() {
        if (server.hasArg("name")) {
          String path = "/" + server.arg("name");
          File f = SD.open(path.c_str());
          if (f) {
            server.streamFile(f, "image/bmp");
            f.close();
            return;
          }
        }
        server.send(404, "text/plain", "File Not Found");
      });
      
      server.on("/api/thumb", HTTP_GET, []() {
        if (server.hasArg("name")) {
          String path = "/.thumbnails/" + server.arg("name");
          File f = SD.open(path.c_str());
          if (f) {
            server.streamFile(f, "image/jpeg");
            f.close();
            return;
          }
        }
        server.send(404, "text/plain", "Thumb Not Found");
      });
      
      server.on("/api/list", HTTP_GET, []() {
        String json = "[";
        File root = SD.open("/");
        if (root && root.isDirectory()) {
          File entry = root.openNextFile();
          bool first = true;
          while (entry) {
            String n = entry.name();
            String lower_n = n;
            lower_n.toLowerCase();
            if (!entry.isDirectory() && !n.startsWith(".") && lower_n.endsWith(".bmp")) {
              String thumbName = n;
              thumbName.replace(".bmp", ".jpg");
              thumbName.replace(".BMP", ".jpg");
              bool hasThumb = SD.exists("/.thumbnails/" + thumbName);
              time_t t = entry.getLastWrite();
              if (!first) json += ",";
              json += "{\"name\":\"" + n + "\",\"size\":" + String(entry.size()) + ",\"time\":" + String(t) + ",\"hasThumb\":" + (hasThumb ? "true" : "false") + "}";
              first = false;
            }
            entry.close();
            entry = root.openNextFile();
          }
        }
        json += "]";
        server.send(200, "application/json", json);
      });
      
      server.on("/api/delete", HTTP_POST, []() {
        if (server.hasArg("filenames")) {
          // 处理多选批量删除
          String names = server.arg("filenames");
          int start = 0;
          int end = names.indexOf(',');
          while (end != -1) {
            String name = names.substring(start, end);
            String path = "/" + name;
            SD.remove(path);
            String thumbName = name; thumbName.replace(".bmp", ".jpg"); thumbName.replace(".BMP", ".jpg");
            if (SD.exists("/.thumbnails/" + thumbName)) SD.remove("/.thumbnails/" + thumbName);
            start = end + 1;
            end = names.indexOf(',', start);
          }
          // 处理最后一个文件
          String name = names.substring(start);
          String path = "/" + name;
          SD.remove(path);
          String thumbName = name; thumbName.replace(".bmp", ".jpg"); thumbName.replace(".BMP", ".jpg");
          if (SD.exists("/.thumbnails/" + thumbName)) SD.remove("/.thumbnails/" + thumbName);
          
          server.send(200, "text/plain", "Deleted Batch");
        } else if (server.hasArg("filename")) {
          // 兼容列表原有单个卡片右侧的 Delete 按钮
          String originalName = server.arg("filename");
          String path = "/" + originalName;
          if (SD.remove(path)) {
            String thumbName = originalName;
            thumbName.replace(".bmp", ".jpg");
            thumbName.replace(".BMP", ".jpg");
            String thumbPath = "/.thumbnails/" + thumbName;
            if (SD.exists(thumbPath)) { SD.remove(thumbPath); }
            server.send(200, "text/plain", "Deleted Single");
          } else {
            server.send(500, "text/plain", "Failed");
          }
        }
      });

      server.on("/api/play_now", HTTP_POST, []() {
        if (server.hasArg("name")) {
          String targetName = server.arg("name");
          
          // 重新构建播放列表以定位该图片的时间排序索引
          std::vector<FileRecord> bmp_files;
          File root = SD.open("/");
          if (root && root.isDirectory()) {
            File entry = root.openNextFile();
            while (entry) {
              if (!entry.isDirectory()) {
                String fileName = entry.name();
                String lowerName = fileName; lowerName.toLowerCase();
                if (!fileName.startsWith(".") && lowerName.endsWith(".bmp")) {
                  FileRecord record;
                  record.path = String("/") + fileName;
                  record.modTime = entry.getLastWrite();
                  bmp_files.push_back(record);
                }
              }
              entry.close();
              entry = root.openNextFile();
            }
          }
          
          // 按时间倒序排列（保持和开机逻辑一致）
          std::sort(bmp_files.begin(), bmp_files.end(), [](const FileRecord& a, const FileRecord& b) {
            return a.modTime > b.modTime;
          });

          // 查找目标文件的索引
          for (size_t i = 0; i < bmp_files.size(); i++) {
            if (bmp_files[i].path == "/" + targetName) {
              image_index = i;
              prefs.putInt("img_idx", image_index);
              break;
            }
          }
          
          web_exit_request = true;
          server.send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
          server.send(400, "text/plain", "Bad Request");
        }
      });
      
      server.on("/api/sysinfo", HTTP_GET, []() {
        float volts = (analogReadMilliVolts(BAT_ADC_PIN) / 1000.0) * VOLTAGE_DIVIDER;
        int pct = getBatteryPercentage();
        float total_gb = SD.totalBytes() / (1024.0 * 1024.0 * 1024.0);
        float used_gb = SD.usedBytes() / (1024.0 * 1024.0 * 1024.0);
        int rssi = WiFi.RSSI();
        String json = "{";
        json += "\"version\":\"" + String(FIRMWARE_VERSION) + "\",";
        json += "\"batt_v\":" + String(volts, 2) + ",";
        json += "\"batt_pct\":" + String(pct) + ",";
        json += "\"sd_used\":" + String(used_gb, 2) + ",";
        json += "\"sd_total\":" + String(total_gb, 2) + ",";
        json += "\"wifi_rssi\":" + String(rssi) + ",";
        json += "\"interval_idx\":" + String(sleep_interval_index);
        json += "}";
        server.send(200, "application/json", json);
      });
      
      server.on("/api/settings", HTTP_POST, []() {
        if (server.hasArg("interval")) {
          int new_idx = server.arg("interval").toInt();
          if (new_idx >= 0 && new_idx <= 7) {
            sleep_interval_index = new_idx;
            prefs.putInt("interval", sleep_interval_index);
          }
        }
        server.send(200, "ok");
      });
      
      // ==========================================
      // 极速流式大文件接收引擎
      // ==========================================
      server.on(
        "/api/upload", HTTP_POST,
        []() {
          server.send(200, "text/plain", "Upload Complete");
        },
        []() {
          HTTPUpload& upload = server.upload();
          static File uploadFile;
          String filename = upload.filename;
          String filepath = "/" + filename;
          if (filename.endsWith(".jpg") || filename.endsWith(".JPG")) {
            filepath = "/.thumbnails/" + filename;
          }
          if (upload.status == UPLOAD_FILE_START) {
            if (SD.exists(filepath)) { SD.remove(filepath); }
            uploadFile = SD.open(filepath, FILE_WRITE);
            Serial.println("[Upload] Start: " + filepath);
          } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (uploadFile) {
              uploadFile.write(upload.buf, upload.currentSize);
            }
          } else if (upload.status == UPLOAD_FILE_END) {
            if (uploadFile) {
              uploadFile.close();
            }
            Serial.println("[Upload] Finished: " + filepath + " (" + String(upload.totalSize / 1024) + " KB)");
          }
        });
      
      server.begin();
    }
    
    unsigned long start_time = millis();
    while (millis() - start_time < (15 * 60 * 1000UL)) {
      if (wifi_connected) { server.handleClient(); }
      
      if (digitalRead(BOOT_BTN) == LOW || web_exit_request) {
        if (digitalRead(BOOT_BTN) == LOW) {
          delay(50);
          if (digitalRead(BOOT_BTN) == LOW) {
            while (digitalRead(BOOT_BTN) == LOW) { delay(10); }
          }
        }
        if (web_exit_request) { delay(500); }
        break;
      }
      delay(2);
    }
    
    if (wifi_connected) { server.close(); }
    WiFi.disconnect(true);
    
    if (web_exit_request) {
      // 在这里不重置为 0，因为如果在 web 里触发了 play_now，
      // image_index 已经是最新的目标了。保留它并在重启后播放即可。
    } else {
       // 自然超时退出的情况，是否归零取决于你的原设计
       // image_index = 0;
       // prefs.putInt("img_idx", image_index);
    }
    ESP.restart();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BOOT_BTN, INPUT_PULLUP);
  SYS_WIDTH = EPD_13IN3E_WIDTH;
  SYS_HEIGHT = EPD_13IN3E_HEIGHT;
  
  prefs.begin("epd_app", false);
  image_index = prefs.getInt("img_idx", 0);
  sleep_interval_index = prefs.getInt("interval", 2);
  bool need_tik = false;
  
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  esp_reset_reason_t reset_reason = esp_reset_reason();
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    long press_start = millis();
    bool is_long_press = false;
    int click_count = 1;
    bool is_pressed = true;
    while (millis() - press_start < 5000) {
      bool current_state = (digitalRead(BOOT_BTN) == LOW);
      if (current_state && !is_pressed) {
        click_count++;
        is_pressed = true;
        delay(30);
      } else if (!current_state && is_pressed) {
        is_pressed = false;
        delay(30);
      }
      if (!current_state && (millis() - press_start > 800)) break;
      if (current_state && click_count == 1 && (millis() - press_start >= 4900)) {
        is_long_press = true;
        break;
      }
      delay(10);
    }
    if (is_long_press) {
      while (digitalRead(BOOT_BTN) == LOW) { delay(10); }
      playSoundOnce(sound_off, sizeof(sound_off));
      clearScreenAndShutdown();
    } else if (click_count >= 2) {
      while (digitalRead(BOOT_BTN) == LOW) { delay(10); }
      playSoundOnce(sound_beep, sizeof(sound_beep));
      checkManagementMode();
    } else {
      need_tik = true;
      image_index++;
      prefs.putInt("img_idx", image_index);
      while (digitalRead(BOOT_BTN) == LOW) { delay(10); }
    }
  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    image_index++;
    prefs.putInt("img_idx", image_index);
    need_tik = true;
  } else if (reset_reason == ESP_RST_SW) {
    need_tik = true;
  } else {
    Serial.println("\r\n========================================");
    Serial.println("SYSTEM BOOT UP (" + String(FIRMWARE_VERSION) + ")");
    USB.begin();
    bool is_usb_connected = false;
    for (int i = 0; i < 100; i++) {
      if (tud_connected()) {
        is_usb_connected = true;
        break;
      }
      delay(10);
    }
    if (is_usb_connected) {
      Serial.println("-> [Alert] PC Connection detected!");
      Serial.println("-> Entering deep sleep to prevent boot interference...");
      Serial.println("========================================\r\n");
      esp_sleep_enable_ext0_wakeup((gpio_num_t)BOOT_BTN, 0);
      esp_sleep_enable_timer_wakeup((uint64_t)INTERVAL_MAP[sleep_interval_index] * 1000000ULL);
      esp_deep_sleep_start();
    } else {
      // 此处不再强行覆盖为 0，防止 /api/play_now 修改的值被覆盖
      // image_index = 0; 
      // prefs.putInt("img_idx", image_index);
      need_tik = true;
    }
  }
  
  if (getBatteryPercentage() < LOW_BAT_THRESHOLD) showStatusIconAndSleep(low_bat_icon_bmp);
  if (need_tik) startTikLoop();
  
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, sdSPI)) {
    if (need_tik) stopTikLoop();
    showStatusIconAndSleep(no_storage_icon_bmp);
  }
  
  std::vector<FileRecord> bmp_files;
  File root = SD.open("/");
  if (root && root.isDirectory()) {
    File entry = root.openNextFile();
    while (entry) {
      if (!entry.isDirectory()) {
        String fileName = entry.name();
        String lowerName = fileName;
        lowerName.toLowerCase();
        if (!fileName.startsWith(".") && lowerName.endsWith(".bmp")) {
          FileRecord record;
          record.path = String("/") + fileName;
          record.modTime = entry.getLastWrite();
          bmp_files.push_back(record);
        }
      }
      entry.close();
      entry = root.openNextFile();
    }
  }
  
  if (bmp_files.empty()) {
    SD.end();
    sdSPI.end();
    if (need_tik) stopTikLoop();
    showStatusIconAndSleep(no_image_icon_bmp);
  }
  
  std::sort(bmp_files.begin(), bmp_files.end(), [](const FileRecord& a, const FileRecord& b) {
    return a.modTime > b.modTime;
  });
  
  image_index = image_index % bmp_files.size();
  String targetFile = bmp_files[image_index].path;
  
  uint32_t bufferSize = (SYS_WIDTH / 2) * SYS_HEIGHT;
  EPD_Buffer = (uint8_t*)ps_malloc(bufferSize);
  if (EPD_Buffer != NULL) {
    memset(EPD_Buffer, 0x11, bufferSize);
    File bmpFile = SD.open(targetFile.c_str());
    if (bmpFile && bmpFile.read() == 'B' && bmpFile.read() == 'M') {
      bmpFile.seek(10);
      uint32_t dataOffset = read32(bmpFile);
      bmpFile.seek(18);
      int32_t img_w = read32(bmpFile);
      int32_t img_h_raw = read32(bmpFile);
      bool bottomUp = (img_h_raw > 0);
      int32_t img_h = abs(img_h_raw);
      bmpFile.seek(28);
      uint16_t depth = read16(bmpFile);
      bmpFile.seek(30);
      uint32_t comp = read32(bmpFile);
      
      if ((depth == 24 || depth == 32) && comp == 0) {
        need_rotate = (img_w < img_h) != (SYS_WIDTH < SYS_HEIGHT);
        bmpFile.seek(dataOffset);
        int bytesPerPixel = depth / 8;
        int padding = (4 - ((img_w * bytesPerPixel) % 4)) % 4;
        uint8_t* rowBuf = (uint8_t*)malloc(img_w * bytesPerPixel);
        if (rowBuf) {
          for (int row = 0; row < img_h; row++) {
            bmpFile.read(rowBuf, img_w * bytesPerPixel);
            if (padding > 0) {
              uint8_t padBuf[3];
              bmpFile.read(padBuf, padding);
            }
            int sy = bottomUp ? (img_h - 1 - row) : row;
            for (int col = 0; col < img_w; col++) {
              uint8_t b = rowBuf[col * bytesPerPixel], g = rowBuf[col * bytesPerPixel + 1], r = rowBuf[col * bytesPerPixel + 2];
              int tx = col, ty = sy;
              if (need_rotate) {
                tx = sy;
                ty = (SYS_HEIGHT - 1) - col;
              }
              if (tx < 0 || tx >= SYS_WIDTH || ty < 0 || ty >= SYS_HEIGHT) continue;
              uint8_t cmd = getNearestE6ColorRGB(r, g, b);
              uint32_t addr = ty * (SYS_WIDTH / 2) + (tx / 2);
              if (tx % 2 == 0) EPD_Buffer[addr] = (EPD_Buffer[addr] & 0x0F) | (cmd << 4);
              else EPD_Buffer[addr] = (EPD_Buffer[addr] & 0xF0) | cmd;
            }
          }
          free(rowBuf);
        }
      }
      bmpFile.close();
    }
    SD.end();
    sdSPI.end();
    
    DEV_Module_Init();
    delay(200);
    EPD_13IN3E_Init();
    EPD_13IN3E_Display(EPD_Buffer);
    EPD_13IN3E_Sleep();
    DEV_Module_Exit();
    free(EPD_Buffer);
  }
  
  if (need_tik) stopTikLoop();
  
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BOOT_BTN, 0);
  esp_sleep_enable_timer_wakeup((uint64_t)INTERVAL_MAP[sleep_interval_index] * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {}