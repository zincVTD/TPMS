# Tire Pressure Monitoring System
Đây là mã nguồn firmware của đề tài TPMS của Võ Thành Danh. Dự án này thiết kế và phát triển một hệ thống mô phỏng hoạt động của ECU TPMS - một ECU có chức năng theo dõi tình trạng áp suất lốp xe trong các xe ô tô. Dự án thực hiện ECU TPMS trực tiếp, tức là sẽ có các cảm biến được gắn trực tiếp vào bên trong các bánh xe. Dữ liệu áp suất thu được sẽ được phát không dây ra bên ngoài và sẽ được thu bởi một bộ nhận trung tâm được đặt trong thân xe. Bộ nhận này sẽ xử lý thông tin nhận được và gửi dữ liệu ra ECU màn hình.
### Chi tiết các thành phần của dự án như sau:
- ADC_BLE: chứa mã nguồn firmware của ESP32C6 dùng để đọc giá trị cảm biến và gửi nó thông qua BLE.
- BLE_SPI: chứa mã nguồn firmware của ESP32C6 dùng để nhận dữ liệu từ BLE và gửi dữ liệu cho STM32 bằng SPI.
- TPMS: chứa mã nguồn firmware của STM32 dùng để nhận dữ liệu từ SPI và gửi nó cho ECU màn hình bằng CAN.
- Monitor: chứa mã nguồn firmware của STM32 dùng để nhận dữ liệu từ CAN và hiển thị chúng ra màn hình OLED SSD1306.
