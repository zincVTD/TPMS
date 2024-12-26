#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

// Định nghĩa BLE
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Định nghĩa chân ADC
#define ADC_PIN 5

BLEClient *pClient;
BLERemoteCharacteristic *pRemoteCharacteristic;
bool connected = false;

void connectToServer() {
  Serial.println("Connecting to BLE server...");
  pClient = BLEDevice::createClient();
  BLEAddress serverAddress("F0:F5:BD:2C:E2:1E"); // Thay bằng địa chỉ BLE của server
  pClient->connect(serverAddress);
  Serial.println("Connected to BLE server");

  // Tìm dịch vụ và characteristic
  BLERemoteService *pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService) {
    pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
    if (pRemoteCharacteristic) {
      connected = true;
      Serial.println("Found characteristic!");
    }
  }
}

void setup() {
  // Cấu hình Serial để debug
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Starting BLE Client...");

  // Khởi tạo BLE
  BLEDevice::init("ESP32C6-ADC-Client");
  connectToServer();

  // Cấu hình chân ADC
  pinMode(ADC_PIN, INPUT);
}

void loop() {
  if (connected) {
    // Đọc giá trị ADC
    uint16_t rawADC = analogRead(ADC_PIN);
    
    // Debug giá trị ADC
    Serial.print("ADC Value: ");
    Serial.println(rawADC);

    // Chuẩn bị dữ liệu dạng 2 byte
    uint8_t adcData[2];
    adcData[0] = (rawADC >> 8) & 0xFF; // Byte cao
    adcData[1] = rawADC & 0xFF;        // Byte thấp

    // Gửi giá trị ADC qua BLE
    pRemoteCharacteristic->writeValue(adcData, 2);
    Serial.println("ADC data sent via BLE");

    delay(1000); // Delay 1 giây
  } else {
    Serial.println("Reconnecting to BLE server...");
    connectToServer();
  }
}
