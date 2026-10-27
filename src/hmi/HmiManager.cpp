#include "HmiManager.h"

bool HmiManager::s_initialized = false;
HardwareSerial* HmiManager::s_serial = &Serial1;

void HmiManager::init() {
#if ENABLE_HMI_UART
  s_serial->begin(HMI_UART_BAUD, SERIAL_8N1, HMI_UART_RX_PIN, HMI_UART_TX_PIN);
  s_initialized = true;

  Serial.printf("[HMI] HMI_UART initialized on Port %d (TX: IO%d, RX: IO%d, Baud: %d)\n",
                HMI_UART_PORT_NUM, HMI_UART_TX_PIN, HMI_UART_RX_PIN, HMI_UART_BAUD);

  // Gửi lệnh xóa màn hình hoặc reset giao diện Nextion/DWIN/Serial
  sendNextionCommand("page 0");
  sendText("\n=== CHATBOT V2 / SMART ORDER READY ===\n");
#else
  s_initialized = false;
#endif
}

bool HmiManager::isAvailable() {
  return s_initialized;
}

void HmiManager::sendNextionCommand(const String& cmd) {
#if ENABLE_HMI_UART
  if (!s_initialized) return;
  s_serial->print(cmd);
  s_serial->write(0xFF);
  s_serial->write(0xFF);
  s_serial->write(0xFF);
#endif
}

void HmiManager::sendText(const String& text) {
#if ENABLE_HMI_UART
  if (!s_initialized) return;
  s_serial->println(text);
#endif
}

void HmiManager::showBoot(const String& deviceId, const String& model) {
#if ENABLE_HMI_UART
  sendNextionCommand("t_title.txt=\"CHATBOT V2 ESP32-S3\"");
  sendNextionCommand("t_dev.txt=\"ID: " + deviceId + "\"");
  sendNextionCommand("t_status.txt=\"Dang khoi dong...\"");
  sendText("[HMI] Boot: " + model + " | ID: " + deviceId);
#endif
}

void HmiManager::showBleAdvertising(const String& bleName, const String& deviceId) {
#if ENABLE_HMI_UART
  sendNextionCommand("t_status.txt=\"Cho ket noi BLE: " + bleName + "\"");
  sendNextionCommand("p_status.pic=1"); // Icon Bluetooth
  sendText("[HMI] BLE Advertising: " + bleName);
#endif
}

void HmiManager::showBleConnected() {
#if ENABLE_HMI_UART
  sendNextionCommand("t_status.txt=\"Da ket noi BLE thanh cong!\"");
  sendNextionCommand("p_status.pic=2"); // Icon Connected
  sendText("[HMI] BLE Connected!");
#endif
}

void HmiManager::showWifiConnecting(int attempt, int maxAttempts) {
#if ENABLE_HMI_UART
  String msg = "Dang ket noi Wi-Fi (" + String(attempt) + "/" + String(maxAttempts) + ")...";
  sendNextionCommand("t_status.txt=\"" + msg + "\"");
  sendText("[HMI] " + msg);
#endif
}

void HmiManager::showWifiConnected(const String& ssid, const String& ipStr, int8_t rssi, uint8_t batteryPct) {
#if ENABLE_HMI_UART
  sendNextionCommand("t_wifi.txt=\"" + ssid + " (" + String(rssi) + "dBm)\"");
  sendNextionCommand("t_ip.txt=\"" + ipStr + "\"");
  sendNextionCommand("t_status.txt=\"Wi-Fi San Sang!\"");
  sendNextionCommand("j_batt.val=" + String(batteryPct));
  sendText("[HMI] Wi-Fi OK: " + ssid + " | IP: " + ipStr + " | Pin: " + String(batteryPct) + "%");
#endif
}

void HmiManager::showWifiError() {
#if ENABLE_HMI_UART
  sendNextionCommand("t_status.txt=\"Loi ket noi Wi-Fi!\"");
  sendText("[HMI] ERROR: Wi-Fi connection failed!");
#endif
}

void HmiManager::showOrderSending(const String& itemName, uint8_t quantity) {
#if ENABLE_HMI_UART
  String msg = "Dang gui don: " + itemName + " x" + String(quantity);
  sendNextionCommand("t_status.txt=\"" + msg + "\"");
  sendText("[HMI] " + msg);
#endif
}

void HmiManager::showOrderSuccess(const String& orderNumber, const String& itemName, uint8_t quantity) {
#if ENABLE_HMI_UART
  sendNextionCommand("t_status.txt=\"Dat mon THANH CONG!\"");
  sendNextionCommand("t_order.txt=\"Don: " + orderNumber + "\"");
  sendNextionCommand("t_item.txt=\"" + itemName + " x" + String(quantity) + "\"");
  sendText("[HMI] SUCCESS: Dat mon " + itemName + " x" + String(quantity) + " | Ma don: " + orderNumber);
#endif
}

void HmiManager::showBillRequest() {
#if ENABLE_HMI_UART
  sendNextionCommand("t_status.txt=\"Dang goi thanh toan...\"");
  sendText("[HMI] Dang yeu cau hoa don...");
#endif
}

void HmiManager::showBillSuccess() {
#if ENABLE_HMI_UART
  sendNextionCommand("t_status.txt=\"Da goi phuc vu tinh tien!\"");
  sendText("[HMI] SUCCESS: Yeu cau thanh toan da gui den he thong!");
#endif
}

void HmiManager::showCancelOrder() {
#if ENABLE_HMI_UART
  sendNextionCommand("t_status.txt=\"Dang yeu cau huy don...\"");
  sendText("[HMI] Dang yeu cau huy don hang...");
#endif
}

void HmiManager::showCancelSuccess() {
#if ENABLE_HMI_UART
  sendNextionCommand("t_status.txt=\"Da HUY don hang thanh cong!\"");
  sendText("[HMI] SUCCESS: Don hang da duoc huy!");
#endif
}

void HmiManager::showBatteryStatus(uint32_t voltageMv, uint8_t percentage) {
#if ENABLE_HMI_UART
  sendNextionCommand("j_batt.val=" + String(percentage));
  sendNextionCommand("t_batt.txt=\"" + String(percentage) + "% (" + String(voltageMv / 1000.0f, 2) + "V)\"");
  sendText("[HMI] Pin 18650/LiPo: " + String(percentage) + "% (" + String(voltageMv) + " mV)");
#endif
}

void HmiManager::showError(const String& message) {
#if ENABLE_HMI_UART
  sendNextionCommand("t_status.txt=\"LOI: " + message + "\"");
  sendText("[HMI] ERROR: " + message);
#endif
}
