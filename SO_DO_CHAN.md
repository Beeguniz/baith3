# Đối chiếu sơ đồ chân Farm House rev 1.0.1

## Xác nhận LED mới nhất

Người dùng xác nhận LED HIGH bật, LOW tắt. Firmware hiện dùng PWM thật 1 kHz/8-bit: LED1 GPIO13/J11, LED2 GPIO12/J13; duty 0 khi khởi động, dừng hoặc lỗi. Không đổi dây hoặc số GPIO theo datasheet N531 tìm trên mạng. Khác biệt số chân IC trong tài liệu vẫn chưa được giải thích, nhưng cực tính điều khiển được chốt theo xác nhận thực tế của người dùng. Mọi ghi chú “LED mô phỏng/chưa chốt cực tính” bên dưới là lịch sử kế hoạch, đã được thay thế. MOTOR2 không tác động.

## Cập nhật triển khai MOTOR1

Cập nhật PWM bơm: GPIO5/IN1 dùng LEDC 1 kHz/8-bit, GPIO17/IN2 giữ LOW. Web chọn 25/50/75/100%, mặc định 100%; dừng là duty 0, không phanh. Mức PWM chưa được hiệu chuẩn thành lưu lượng. MOTOR2 không tác động.

Đã tích hợp MOTOR1 thật: GPIO5 → IN1, GPIO17 → IN2 của U9 A4950ELJTR. Chạy một chiều bằng HIGH/LOW; dừng bằng LOW/LOW (coast/thả trôi). LOW/LOW được thiết lập trước Wi-Fi. Theo [datasheet Allegro, trang 4–5](https://www.allegromicro.com/-/media/files/datasheets/a4950-datasheet.ashx), tổ hợp HIGH/HIGH là phanh điện nhưng không dùng trong bản này. Chiều quay cơ học chưa kiểm tra. LED còn mô phỏng do chưa xác nhận N531 thực tế; MOTOR2 không tác động. Đã build, chưa nạp board. Ghi chú này cập nhật trạng thái triển khai, ưu tiên so với mô tả kế hoạch cũ phía dưới.

> **RÀNG BUỘC BẮT BUỘC: TUYỆT ĐỐI KHÔNG CODE HAY ĐỤNG CHẠM VÀO MOTOR2.** GPIO23, GPIO22, đầu nối J10 và mạch MOTOR2 nằm ngoài phạm vi thực hiện. Không khởi tạo/cấu hình/đọc/ghi các chân này, kể cả đặt LOW, tắt tải, PWM hoặc thử nghiệm; không đấu nối, tháo dây hay tác động phần cứng MOTOR2. Thông tin MOTOR2 bên dưới chỉ để nhận diện và tránh sử dụng.

## Nguồn và mức độ xác nhận

Tài liệu đối chiếu: [farm_house_schematic_rev1.0.1.pdf](../farm_house_schematic_rev1.0.1.pdf), tiêu đề Farm House Demo, Long Hung Tech Ltd., revision 1.0.1, ngày 2025-02-05.

Đã đọc chữ và đối chiếu hình vẽ trang 2, 3, 4, gồm cả vùng chân U1 được phóng lớn. Bảng dưới ghi **kết nối theo PDF**, chưa xác nhận bằng đo thông mạch hoặc chạy trên board thực tế. Tài liệu PDF được dùng làm nguồn sơ đồ; kế hoạch dùng nút web vẫn theo yêu cầu của người dùng.

| Trang PDF | Nội dung | Vai trò khi đối chiếu |
| --- | --- | --- |
| 1/5 | Sơ đồ phân cấp | Chia khối nguồn, UNO, ESP32 |
| 2/5 | Farm House Power | Nguồn VDC, +5V, +3V3, +5VA và tín hiệu đo nguồn |
| 3/5 | Farm House Controller Uno ATMEGA328 | Thiết bị thuộc Arduino UNO, không lấy số chân này làm GPIO ESP32 |
| 4/5 | Farm House Controller ESP32 | Nguồn chính để lập bảng GPIO dưới đây |
| 5/5 | Sheet con không có mạch đáng kể | Không bổ sung ánh xạ GPIO |

U1 ở trang 4 là **ESP32-DEVKITC-32E**. Ký hiệu `J2_15`, `J3_10` trong bảng là mã chân trên symbol U1, không phải đầu nối tải J2/J3 ở khối nguồn. Số chân đầu nối được chép theo schematic, không suy ra hướng trái/phải khi nhìn board thật.

## 1. Bảng chân ESP32

| Thiết bị / chức năng | GPIO hoặc tên chân U1 | Chân symbol U1 | Tên tín hiệu trên sơ đồ | Đích kết nối |
| --- | --- | --- | --- | --- |
| LED công suất, khối **ARRAY_LED1** | **GPIO13** | J2_15 | `ARRAY_LED3` | U11 N531 → Q10 30N06 → J11 |
| LED công suất, khối **ARRAY_LED2** | **GPIO12** | J2_13 | `ARRAY_LED4` | U12 N531 → Q11 30N06 → J13 |
| Động cơ 1, đầu vào 1 | **GPIO5** | J3_10 | `MOTOR1_INT1` | R37 22Ω → IN1, chân 3 U9 A4950ELJTR |
| Động cơ 1, đầu vào 2 | **GPIO17** | J3_11 | `MOTOR1_INT2` | R53 22Ω → IN2, chân 2 U9 |
| Động cơ 2, đầu vào 1 | **GPIO23** | J3_2 | `MOTOR2_INT1` | R33 22Ω → IN1, chân 3 U3 A4950ELJTR |
| Động cơ 2, đầu vào 2 | **GPIO22** | J3_3 | `MOTOR2_INT2` | R34 22Ω → IN2, chân 2 U3 |
| DHT11 | **GPIO14** | J2_12 | `DATA_RH` | R28 22Ω → J1 chân 2 |
| I²C SDA | **GPIO33** | J2_8 | `SDA` | LCD J8 chân 2; OLED J4 chân 2; nối sang sheet UNO |
| I²C SCL | **GPIO32** | J2_7 | `SCL` | LCD J8 chân 1; OLED J4 chân 1; nối sang sheet UNO |
| LED báo đỏ | GPIO26 | J2_10 | `LED_R` | R14 → Q6 MMBT3904 → D3 |
| LED báo xanh | GPIO27 | J2_11 | `LED_G` | R17 → Q12 MMBT3904 → D16 |
| Buzzer | **GPIO21** | J3_6 | `BUZZ` | R4 → Q2 MMBT3904 → BZ1 |
| Biến trở RV1 | SENSOR_VN (GPIO39) | J2_4 | `ADC_IN` | Con chạy RV1 chân 2; hai đầu RV1 nối +3V3/GNDD |
| Đo nguồn vào | GPIO34 | J2_5 | `VIN_ADC` / `ADC_24` | Cầu chia R1 100kΩ, R2 10kΩ ở trang 2 |
| Đo pin | GPIO35 | J2_6 | `BAT_ADC` / `ADC_BAT` | Cầu chia R10 100kΩ, R11 10kΩ ở trang 2 |
| ADC mở rộng | GPIO0 | J3_14 | `ADC_E1` | R44 22Ω → J18 chân 2 |
| Nhãn nút chức năng | SENSOR_VP (GPIO36) | J2_3 | `FUNC_BUT` | Có nhãn tại U1, chưa thấy mạch nút/đầu nối tương ứng trong PDF |
| RS485 nhận | GPIO4 | J3_13 | `RS485_RXD` | RN3 → RO chân 1 U10 MAX485E |
| RS485 phát | GPIO2 | J3_15 | `RS485_TXD` | RN3 → DI chân 4 U10 MAX485E |
| RS485 điều khiển hướng | GPIO16 | J3_12 | `DE_RS485` | RN3 → /RE chân 2 và DE chân 3 U10; nhánh Q1 |
| UART0 phát | TXD0 | J3_4 | `TXD0` | Đường có nhãn tại U1 |
| UART0 nhận | RXD0 | J3_5 | `RXD0` | Đường có nhãn tại U1 |

Trong symbol, SENSOR_VP/VN được ghi bằng tên chức năng thay vì IO36/IO39; bảng giữ cả tên để dễ đối chiếu. Các chân GPIO25, GPIO18, GPIO19, GPIO15 không thể hiện kết nối đến thiết bị ở trang 4; điều này chưa đủ để kết luận chúng sẵn dùng trên mọi phiên bản board. Các chân SD2/SD3/CMD/SD1/SD0/CLK không được chọn cho bài này.

**Không dùng mặc định SDA=21, SCL=22 cho board này:** sơ đồ đưa I²C ra GPIO33/32, còn GPIO21 nối buzzer và GPIO22 nối đầu vào động cơ 2.

## 2. Các đầu nối cần kiểm tra trực tiếp

### 2.1 LED công suất

| Khối in trong trang 4 | Đầu nối | Chân 1 | Chân 2 | Chuỗi điều khiển |
| --- | --- | --- | --- | --- |
| ARRAY_LED1 | **J11** | VDC | Nhánh tải về drain Q10 | GPIO13 → net ARRAY_LED3 → U11 → Q10 |
| ARRAY_LED2 | **J13** | VDC | Nhánh tải về drain Q11 | GPIO12 → net ARRAY_LED4 → U12 → Q11 |

Ở mỗi kênh, U11/U12 được ghi **N531**, Q10/Q11 được ghi **30N06**, source MOSFET nối GNDD. D9/D10 được ghi **DSK115**, mắc giữa nhánh tải và VDC. Chân 2 của J11/J13 là điểm đóng cắt phía âm tải, **không phải GNDD nối cố định**.

Tên khối và tên net không trùng số: **ARRAY_LED1 ↔ ARRAY_LED3**, **ARRAY_LED2 ↔ ARRAY_LED4**. Đây là cách ghi trong PDF, không tự đổi lại số khi viết firmware. Cần xem nhãn silkscreen/đo thông mạch để xác nhận vị trí thực tế của J11 và J13.

Sơ đồ chưa đủ để kết luận cực tính logic của N531 hoặc trạng thái tải khi GPIO thả nổi. Trước khi chốt `LOAD_ON_LEVEL`/`LOAD_OFF_LEVEL`, cần xác minh linh kiện lắp thực tế, tài liệu driver và phép thử có kiểm soát. Không coi `LOW` là tắt chỉ dựa vào tên net.

### 2.2 Hai kênh động cơ DC

| Kênh | Đầu nối tải | Chân 1 | Chân 2 | IC công suất | GPIO điều khiển |
| --- | --- | --- | --- | --- | --- |
| DC MOTOR 1 | **J9** | OUT1 chân 6 U9 | OUT2 chân 8 U9 | A4950ELJTR | IN1=5, IN2=17 |
| DC MOTOR 2 | **J10** | OUT1 chân 6 U3 | OUT2 chân 8 U3 | A4950ELJTR | IN1=23, IN2=22 |

Cả hai IC lấy VBB từ **VDC**, VREF từ **+5VA**, có điện trở sense 0,2Ω (R63 ở U9, R32 ở U3). Hai chân đầu nối động cơ đều là đầu ra cầu H; không tự coi một chân là GND. Ký hiệu `MOTOR01` ở cả J9/J10 không có nghĩa hai đầu nối là cùng một kênh.

Nếu chọn máy bơm cho bài thực hành, chỉ xem xét kênh MOTOR1 tại J9; cần xác nhận điện áp/dòng phù hợp và bảng điều khiển IN1/IN2 của A4950 trước khi chọn cách dừng/PWM. Không sử dụng hoặc tác động MOTOR2/J10. Bảng trên chỉ xác định dây nối, chưa chọn chiều quay hay chế độ phanh/thả trôi.

### 2.3 Cảm biến, màn hình và mở rộng

| Đầu nối | Chân 1 | Chân 2 | Chân 3 | Chân 4 |
| --- | --- | --- | --- | --- |
| **J1 — DHT11 ESP32** | +5V | DATA_RH qua R28 đến GPIO14 | GNDD | — |
| **J8 — LCD** | SCL qua R30 đến GPIO32 | SDA qua R26 đến GPIO33 | GNDD | +5V |
| **J4 — OLED** | SCL qua R36 đến GPIO32 | SDA qua R29 đến GPIO33 | GNDD | +3V3 |
| **J18 — EXTERNAL_ADC3** | GNDD | ADC_E1 qua R44 đến GPIO0 | +3V3 | — |
| **J5 — RS485** | RSA, nhánh A qua F3 | RSB, nhánh B qua F2 | GNDD | — |

**Điểm cần kiểm tra về mức điện áp:** trang 4 vẽ R27 3,3kΩ kéo DATA_RH lên +5V; trang 3 vẽ R24/R25 3,3kΩ kéo SDA/SCL lên +5V và các net này nối sang trang ESP32. Không thấy tầng chuyển mức trên những đường đó trong bản vẽ. Cần kiểm tra board thực tế, điện áp đường tín hiệu và yêu cầu điện áp của ESP32 trước khi vận hành; không mặc định điện trở nối tiếp 22Ω là mạch chuyển mức.

## 3. Phần thuộc Arduino UNO, tránh nhầm với ESP32

Các kết nối sau nằm ở **trang 3**, không phải chân ESP32 ở trang 4:

| Thiết bị | Chân Arduino UNO | Net / đầu nối |
| --- | --- | --- |
| DHT11 phía UNO | D7 | DATA_RH2, J19 |
| Cảm biến mưa YL-83 | A2 | ADC-U1, J15 chân 4; chân 3 J15 đánh dấu không nối |
| Servo SG90 | D8 | PWM_SG, J16 chân 3 |
| Khối ARRAY_LED4 phía UNO | D3 | PWM_LED_U1, U5/Q7, J12 |
| Khối ARRAY_LED3 phía UNO | D5 | PWM_LED_U2, U6/Q8, J6 |
| Heater | D13 | PWM_HEATER, U8/Q9, J7 |

Vì vậy, **PDF không thể hiện cảm biến mưa nối trực tiếp đến một GPIO ESP32**. Không gán cảm biến mưa vào GPIO34/35: hai chân đó đã nối đo nguồn theo sơ đồ. Nếu cần dùng dữ liệu mưa, phải xác định cách trao đổi với UNO hoặc phương án đấu lại riêng.

## 4. Nguồn cấp liên quan đến tải

Theo trang 2, VDC nhận nguồn qua D4 từ đường `+12C` hoặc qua D5 từ `+BATT`. Khối tạo `+12C` có tiêu đề “DCDC 12.6VDC 3A”; điện áp VDC thực tế còn phụ thuộc nguồn đang dùng và sụt áp trên diode. **Không ghi VDC là 5V hoặc mặc định đúng 12,0V.**

Trang 4 dùng VDC cho các kênh LED công suất và nguồn động cơ. ESP32 nhận `+VCC_5V` vào U1 J2_19, lấy từ nhánh +5V qua mạng lọc. Các điểm GNDD trong bản vẽ cùng tên mass.

Đây là mô tả mạng nguồn theo sơ đồ, chưa xác nhận điện áp đo thực tế hoặc khả năng cấp dòng liên tục của toàn bộ board.

## 5. Cấu hình dự kiến cho TH03 để bạn duyệt

### Cập nhật phạm vi đã xác nhận

Người dùng yêu cầu điều khiển **cả LED1 và LED2, cùng MOTOR1** bằng ba phần riêng trên web. Cấu hình hiện tại bao gồm J11/GPIO13, J13/GPIO12 và J9/GPIO5+GPIO17. Không dùng MOTOR2.

UNO đảm nhiệm đọc cảm biến và truyền dữ liệu qua I²C, với dây nối do người dùng xác nhận:

| Tín hiệu | UNO R3 | ESP32 |
| --- | --- | --- |
| SDA | A4 | GPIO33 |
| SCL | A5 | GPIO32 |
| Mass chung | GND | GND |

Theo thông tin code hiện tại do người dùng cung cấp, **ESP32 là master**, yêu cầu **4 byte** từ **UNO slave tại địa chỉ `0x08` mỗi 2 giây**. UNO phản hồi khi được yêu cầu.

Người dùng không có code UNO và đã chọn quy ước giải mã tạm thời: byte 0–1 là nhiệt độ × 10, byte 2–3 là độ ẩm × 10; mỗi cặp ghép byte cao trước rồi chia 10. Ví dụ `01 05 01 D6` → **26,1°C và 47,0%RH**. Đây là **giả định triển khai được chấp thuận**, chưa được xác minh từ firmware UNO. Tạm tính không dấu; cách biểu diễn nhiệt độ âm hoặc mã lỗi chưa rõ. Xem công thức và checklist bước 1 trong [README.md](README.md).

GPIO14/DHT11 trong bảng sơ đồ chỉ là thông tin phần cứng, không phải cảm biến sẽ được ESP32 đọc trực tiếp trong phương án đã chốt. Dùng PlatformIO, bỏ phần ngắt nút vật lý theo xác nhận của người dùng. Đã tạo firmware nền nhận I²C trong `src/main.cpp`; chưa triển khai điều khiển tải, chưa build hoặc nạp lên board.

Ánh xạ đã chọn cho triển khai:

| Chức năng | Chân ESP32 | Đầu nối |
| --- | --- | --- |
| LED1 | GPIO13 | J11 |
| LED2 | GPIO12 | J13 |
| MOTOR1 IN1 / IN2 | GPIO5 / GPIO17 | J9 |
| SDA / SCL nhận dữ liệu UNO | GPIO33 / GPIO32 | UNO A4 / A5 |

Chưa chốt cực tính ON/OFF của driver LED, tổ hợp dừng MOTOR1 hoặc thông số PWM. Nút BẬT/TẮT nằm trên web, không chiếm GPIO. Không cấu hình đọc DHT trực tiếp ở GPIO14 trong phương án này.

Nếu chọn bơm nối **J9 (MOTOR1)**, dùng cặp GPIO5/GPIO17. Kênh này không dùng cách điều khiển một chân giống kênh J11. **Không chọn J10/MOTOR2; không đưa GPIO23/GPIO22 vào code, kể cả phần khởi tạo hoặc tắt tải.**

README trước đó chưa chốt GPIO cụ thể. File này bổ sung ánh xạ theo tài liệu để kiểm tra trước khi viết firmware, không xác nhận rằng tải đã được thử an toàn.

## 6. Checklist xác nhận trên mô hình

- [ ] Board thực tế là đúng phiên bản hoặc có cùng đường nối với rev 1.0.1.
- [ ] Chọn tải và ghi đầu nối thực tế: J11 / J13 / J9. Tuyệt đối không chọn hoặc tác động J10/MOTOR2.
- [ ] Đối chiếu chân 1/2 theo nhãn board, không suy ra hướng cắm từ cách đặt symbol trong PDF.
- [ ] Khi đã ngắt nguồn, kiểm tra đường GPIO → driver → đầu nối tải nếu cần.
- [ ] Xác nhận mã linh kiện thực tế N531/30N06 hoặc A4950 và cách đặt đầu ra tắt.
- [ ] Kiểm tra DATA_RH và SDA/SCL do sơ đồ thể hiện kéo lên +5V.
- [ ] Xác nhận nguồn VDC thực đo phù hợp tải và tải tắt khi cấp nguồn/reset.
- [ ] Giảng viên kiểm tra đường công suất trước khi cấp điện chạy tải theo yêu cầu bài thực hành.
- [ ] Giữ nút điều khiển trên web; không coi nhãn FUNC_BUT chưa có mạch là nút đã đấu sẵn.

**Kết luận đối chiếu PDF:** GPIO13/J11 hoặc GPIO12/J13 là hai kênh LED công suất của ESP32; GPIO5/17/J9 và GPIO23/22/J10 là hai kênh động cơ; DHT11 dùng GPIO14/J1. Vị trí đầu nối, cực tính điều khiển và mức điện áp cần được xác nhận trên board thực tế trước khi chạy.
