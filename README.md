# Báo cáo hệ nhúng - Game xe vượt chướng ngại vật
- Lớp: Hệ nhúng - 143800
- Thành viên:
    - Trần Phúc Mạnh Linh - 20200352
    - Nguyễn Duy Long - 20204580
- Thiết bị: Bộ KIT 8051 PRO với vi điều khiển STC89C52

## Giới thiệu
<!-- %<Khái quát về trò chơi>% -->
**Tổng quan về Trò chơi:**

Trò chơi này chạy trên một vi điều khiển 8051 và là một trò chơi vượt chướng ngại vật với người chơi điều khiển một chiếc xe hình biểu tượng `'>'`, cố gắng di chuyển qua một bản đồ thay đổi liên tục có đầy chướng ngại vật. Mục tiêu là tránh va chạm với chướng ngại vật để tối đa hóa điểm của người chơi. Trò chơi có chướng ngại vật và nhiều loại item được biểu thị bằng các ký hiệu khác nhau, bao gồm 'X' cho item xóa bỏ tường gần nhất trên đường đua, 'L' cho item làm chậm chiếc xe và 'F' cho item tăng tốc.

## Phân công công việc
- Trần Phúc Mạnh Linh:
    - Phụ trách phần điều khiển các thiết bị ngoại vi như LCD 16x2, Phím S1-S16, Loa.
    - Phụ trách phần điều hướng xe, xử lý va chạm và sinh chướng ngại vật trong game.
    - Thiết kế giao diện đồ họa cho game.

- Nguyễn Duy Long:
    - Phụ trách phần cơ chế tăng tốc, giảm tốc, xuyên tường.
    - Phụ trách phần sinh vật phẩm tăng tốc, giảm tốc, xuyên tường.
    - Phụ trách phần giao diện kết thúc game.

## Hướng dẫn chơi
<!-- ### Luật chơi
%<Luật chơi>%

### Cách chơi
%<Cách chơi>% -->

**Cách chơi:**

1. **Điều khiển:**
   - Nhấn nút ở phía bên phải (`P1_3`) để di chuyển xe lên.
   - Nhấn nút ở phía bên trái (`P1_0`) để di chuyển xe xuống.

2. **Mục tiêu:**
   - Điều khiển xe qua bản đồ thay đổi liên tục mà không va chạm với chướng ngại vật.
   - Kiếm điểm cho mỗi hành trình thành công.

3. **Cơ chế trò chơi:**
   - Bản đồ trò chơi thay đổi liên tục liên tục từ phải qua trái, xuất hiện các chướng ngại vật hoặc item mới.
   - Va chạm với chướng ngại vật dẫn đến GameOver, với một số item ảnh hưởng đến tốc độ di chuyển của xe hoặc kích hoạt sự kiện cụ thể.
   - Trò chơi tiếp tục cho đến khi có va chạm với chướng ngại vật, lúc đó điểm của người chơi được hiển thị.

**Chức năng của Trò chơi:**

1. **Tạo Bản đồ:**
   - Chướng ngại vật và item được tạo ngẫu nhiên, mang lại độ đa dạng trong mỗi lần chơi.
   - Các loại item và chướng ngại vật khác nhau thêm phức tạp và đa dạng vào lối chơi.

2. **Xử lý Va chạm:**
   - Phát hiện va chạm với chướng ngại vật hoặc item, ảnh hưởng đến trạng thái trò chơi.
   - Trò chơi phản ứng với va chạm bằng cách điều chỉnh tốc độ xe hoặc kích hoạt các sự kiện cụ thể.

3. **Giao diện Người dùng:**
   - Trạng thái của trò chơi, bao gồm điểm số và các sự kiện có thể, được hiển thị trên một màn hình LCD 16x2.
   - Các thông báo thông tin, như màn hình thất bại, được hiển thị cho người chơi.

4. **Âm thanh Phản hồi:**
   - Triển khai âm thanh phản hồi, như tiếng bíp ngắn hoặc bíp dài, để tăng cường trải nghiệm chơi game.


## Chi tiết thuật toán
<!-- %<Trò chơi được xây dựng trên công nghệ gì>%

%<Thuật toán>%

%<Các hàm chính>% -->
Thuật toán được xây dựng trên nền tảng ngôn ngữ C và các thư viện hỗ trợ của vi điều khiển 8051. Trò chơi được xây dựng trên một mô hình bản đồ thay đổi liên tục liên tục, trong đó mỗi hàng của bản đồ được lưu trữ trong một mảng 1 chiều. Mỗi phần tử của mảng đại diện cho một ô trên bản đồ, với các giá trị khác nhau để biểu thị các loại chướng ngại vật và item khác nhau. Trạng thái của trò chơi được lưu trữ trong các biến toàn cục, bao gồm điểm số, tốc độ xe, vị trí của xe, v.v. Các hàm được sử dụng để xử lý các sự kiện cụ thể, bao gồm xử lý va chạm, tạo bản đồ, v.v. Trò chơi được thực hiện trong một vòng lặp vô hạn, trong đó trạng thái của trò chơi được cập nhật và bản đồ được vẽ lên màn hình LCD.


1. **Các lệnh Include:**
   - `#include <mcs51/8051.h>`: Bao gồm tệp header cụ thể cho vi điều khiển 8051.
   - `"lcd1602.h"`: Bao gồm một tệp header để giao tiếp với màn hình LCD 16x2.
   - Các thư viện tiêu chuẩn của C như `<stdio.h>` và `<stdlib.h>` cũng được bao gồm.

2. **Định nghĩa Macro:**
   - Định nghĩa cho các chân phần cứng (`SPEAKER`, `LED7SEG_DATA`) và giá trị của bộ đếm thời gian (`TH0_50ms`, `TL0_50ms`, `TH1_ms`, `TL1_ms`).

3. **Biến Toàn cục:**
   - Biến để lưu trạng thái trò chơi (`map`, `carPosition`, `typeOb1`, `typeOb2`, `typeOb3`, v.v.).
   - Biến liên quan đến cơ chế trò chơi (`score`, `soundFreq`, `soundEnable`, `isPlaying`, `speed`, `timeSpeed`, `carIcon`).

4. **Các Hàm:**
   - `printInt`: Chuyển đổi một số nguyên thành chuỗi để hiển thị trên màn hình LCD.
   - `randomNewObstacle`: Tạo ra một mẫu chướng ngại vật mới cho trò chơi.
   - `handleImpact`, `handleImpactTypeOb1`, `handleImpactTypeOb2`, `handleImpactTypeOb3`: Các hàm để kiểm tra va chạm.
   - `displayFailure`: Hiển thị thông báo thất bại trên màn hình LCD.
   - `moveMapForward`: Di chuyển bản đồ trò chơi về phía trước và xử lý chướng ngại vật.
   - `initMap`: Khởi tạo bản đồ trò chơi với chướng ngại vật ngẫu nhiên.
   - `renderMap`: Vẽ bản đồ trò chơi lên màn hình LCD.

5. **Hàm Main:**
   - Khởi tạo phần cứng và màn hình LCD.
   - Khởi tạo bản đồ trò chơi.
   - Bước vào một vòng lặp trong đó trò chơi chạy.
   - Xử lý đầu vào của người chơi (nhấn nút).
   - Cập nhật trạng thái trò chơi và kiểm tra va chạm.
   - Vẽ bản đồ trò chơi lên màn hình LCD.
   - Trò chơi tiếp tục cho đến khi đáp ứng điều kiện thất bại.


Tóm lại, trò chơi cung cấp một trải nghiệm đơn giản và hấp dẫn, thách thức người chơi thể hiện sự chính xác và kỹ năng phản ứng của mình. Sự kết hợp giữa các loại chướng ngại vật và sự kiện khác nhau thêm đa dạng vào lối chơi, làm cho mỗi lần chơi trở nên độc đáo. Người chơi cần có chiến thuật để đạt được điểm số cao, đồng thời liên tục thích ứng với môi trường trò chơi đang thay đổi.
