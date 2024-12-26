#include <SPI.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Định nghĩa BLE
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Định nghĩa chân SPI
#define SPI_SCK_PIN 2
#define SPI_MISO_PIN 1
#define SPI_MOSI_PIN 0
#define SPI_CS_PIN 14 // Chân CS của Slave (tương ứng FSPICS2)

// SPI settings
SPIClass *spi = NULL;
const uint32_t SPI_CLOCK = 50000;    // Tốc độ SPI 50 kHz
const uint8_t SPI_MODE = SPI_MODE0;  // CPOL = 0, CPHA = 0 (SPI_MODE0)
const uint8_t SPI_BIT_ORDER = MSBFIRST;

// Bộ đệm dữ liệu
const int BUFFER_SIZE = 6;
uint8_t tx_buffer[BUFFER_SIZE] = {0xFF, 0x00, 0x00, 0x00, 0xF0, 0xFF};
bool dataReceived = false;

// BLE setup
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Callback khi kết nối BLE
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

// Xử lý dữ liệu nhận được từ BLE
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value.length() == 2) { // Chỉ xử lý nếu nhận đúng 2 byte
      uint16_t rawADC = (value[0] << 8) | value[1]; // Kết hợp thành số 16-bit

      // Chuyển đổi giá trị từ 0-4096 sang 0-72.5
      float scaledValue = (rawADC / 4096.0) * 72.5;
      tx_buffer[0] = static_cast<uint8_t>(scaledValue); // Lưu vào byte 0 của tx_buffer

      // Cập nhật byte 4 và byte 5 theo yêu cầu
      tx_buffer[4] = 0xF0; // 4 bit đầu là 1111, 4 bit sau là 0000

      dataReceived = true;

      // Debug
      Serial.println("Data received and processed via BLE:");
      Serial.printf("Raw ADC: %u\n", rawADC);
      Serial.printf("Scaled Value: %.2f\n", scaledValue);
      Serial.print("Buffer to send: ");
      for (int i = 0; i < BUFFER_SIZE; i++) {
        Serial.printf("0x%02X ", tx_buffer[i]);
      }
      Serial.println();
    } else {
      Serial.println("Invalid data length received via BLE.");
    }
  }
};

void sendSPIData() {
  // Kích hoạt Slave (CS Low)
  digitalWrite(SPI_CS_PIN, LOW);
  delayMicroseconds(1); // Đợi Slave sẵn sàng

  // Gửi từng byte qua SPI
  for (int i = 0; i < BUFFER_SIZE; i++) {
    spi->transfer(tx_buffer[i]);
  }

  // Kết thúc truyền (CS High)
  digitalWrite(SPI_CS_PIN, HIGH);

  // Debug: In dữ liệu đã gửi
  Serial.print("Sent via SPI: ");
  for (int i = 0; i < BUFFER_SIZE; i++) {
    Serial.printf("0x%02X ", tx_buffer[i]);
  }
  Serial.println();
}

void setup() {
  // Cấu hình chân CS làm OUTPUT và đặt HIGH mặc định
  pinMode(SPI_CS_PIN, OUTPUT);
  digitalWrite(SPI_CS_PIN, HIGH);

  // Khởi tạo Serial để debug
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Initializing BLE...");

  // Khởi tạo BLE
  BLEDevice::init("ESP32C6-BLE-SPI");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Tạo Service và Characteristic
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start BLE service
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->start();
  Serial.println("BLE initialized");

  // Khởi tạo SPI
  spi = new SPIClass(FSPI);
  spi->begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_CS_PIN);
  spi->beginTransaction(SPISettings(SPI_CLOCK, SPI_BIT_ORDER, SPI_MODE));
  Serial.println("SPI Master Initialized");
}

void loop() {
  // Gửi dữ liệu qua SPI nếu có dữ liệu BLE mới
  if (dataReceived) {
    sendSPIData();
    dataReceived = false; // Reset cờ
  }

  delay(100); // Giảm tải CPU
}
