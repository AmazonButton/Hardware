#include "TftDisplayManager.h"

bool TftDisplayManager::s_initialized = false;

void TftDisplayManager::init() {
#if ENABLE_TFT_SPI
  // Cảnh báo an toàn phần cứng từ sơ đồ mạch:
  // "Dùng công tắc nối với 2 chân này, cắm pin trước rồi mới được bật công tắc,
  // cơ chế bảo vệ tải vì cắm pin trước rồi mới cấp lại TFT SPI"
  Serial.println("[TFT_SPI] ========================================================");
  Serial.println("[TFT_SPI] ⚠️  CHU Y AN TOAN BO MACH CHATBOT V2:");
  Serial.println("[TFT_SPI]     Cam pin 18650/LiPo vao B- B+ truoc roi moi bat cong tac");
  Serial.println("[TFT_SPI]     de bao ve mach tai va nguon cap cho cong TFT SPI!");
  Serial.println("[TFT_SPI] ========================================================");

  // Khởi tạo các chân điều khiển SPI
  pinMode(PIN_TFT_CS, OUTPUT);
  digitalWrite(PIN_TFT_CS, HIGH);

  pinMode(PIN_TFT_DC, OUTPUT);
  digitalWrite(PIN_TFT_DC, HIGH);

  pinMode(PIN_TFT_RST, OUTPUT);
  digitalWrite(PIN_TFT_RST, HIGH);

#ifdef PIN_TFT_BLK
  pinMode(PIN_TFT_BLK, OUTPUT);
  digitalWrite(PIN_TFT_BLK, HIGH); // Bật đèn nền
#endif

  // Khởi tạo SPI bus của ESP32-S3 (FSPI)
  SPI.begin(PIN_TFT_SCK, PIN_TFT_MISO, PIN_TFT_MOSI, PIN_TFT_CS);

  Serial.printf("[TFT_SPI] SPI Bus initialized: SCK: IO%d, MOSI: IO%d, MISO: IO%d, CS: IO%d, DC: IO%d\n",
                PIN_TFT_SCK, PIN_TFT_MOSI, PIN_TFT_MISO, PIN_TFT_CS, PIN_TFT_DC);

  s_initialized = true;
#else
  s_initialized = false;
#endif
}

bool TftDisplayManager::isAvailable() {
  return s_initialized;
}
