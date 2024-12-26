# Tire Pressure Monitoring System
Đây là mã nguồn firmware của đề tài TPMS của Võ Thành Danh. Chi tiết như sau:
- ADC_BLE: chứa mã nguồn firmware của ESP32C6 dùng để đọc giá trị cảm biến và gửi nó thông qua BLE.
- BLE_SPI: chứa mã nguồn firmware của ESP32C6 dùng để nhận dữ liệu từ BLE và gửi dữ liệu cho STM32 bằng SPI.
- TPMS: chứa mã nguồn firmware của STM32 dùng để nhận dữ liệu từ SPI và gửi nó cho ECU màn hình bằng CAN.
- Monitor: chứa mã nguồn firmware của STM32 dùng để nhận dữ liệu từ CAN và hiển thị chúng ra màn hình OLED SSD1306.
