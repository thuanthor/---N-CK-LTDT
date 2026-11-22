# HƯỚNG DẪN SỬ DỤNG CHƯƠNG TRÌNH SHIPPERGRAPH

*(Tài liệu thuộc thư mục `Document/` phục vụ nộp đồ án môn Lý thuyết Đồ thị và Ứng dụng — Trường Đại học Sư phạm TP. Hồ Chí Minh)*

---

## 1. Yêu cầu hệ thống & Khởi chạy chương trình

- **Hệ điều hành:** Windows 10 / 11 (64-bit).
- **Cài đặt phụ thuộc:** **Không cần** cài Python, Node.js hay kết nối Internet khi chạy bản `Release`.
- **Cách mở chương trình:**
  1. Trong thư mục gốc dự án: nhấp đúp **`MO-GIAO-DIEN.cmd`** (hoặc trong thư mục `Release/`: nhấp đúp **`CHAY-CHUONG-TRINH.cmd`**).
  2. Một cửa sổ dòng lệnh xuất hiện báo địa chỉ máy chủ nội bộ (mặc định `http://127.0.0.1:8080/`) và tự động mở trình duyệt Web.
  3. **Lưu ý:** Giữ cửa sổ dòng lệnh mở trong suốt quá trình sử dụng. Khi dùng xong, nhấn `Ctrl + C` trong cửa sổ dòng lệnh để tắt máy chủ.

---

## 2. Hướng dẫn sử dụng 5 Phân hệ trên Giao diện Web

### 2.1. Thanh bên trái: Dữ liệu & Ràng buộc
- **Bộ dữ liệu mẫu:**
  - `Mẫu 14 đơn chuẩn`: Bộ dữ liệu đối chiếu chuẩn (với tải `25 kg`, ca `240 phút`, hệ số đường `1.0` cho kết quả **4 shipper, 157,88 km**).
  - `Mẫu TP.HCM (18 điểm)`: Bộ dữ liệu gồm 18 địa điểm quen thuộc tại TP.HCM (ĐH Sư Phạm CS1 & CS2, Chợ Bến Thành, Bưu điện TP, Sân bay Tân Sơn Nhất...).
- **Ràng buộc bài toán:**
  - `Tải tối đa / người (kg)`: Tổng khối lượng các kiện hàng một shipper chở trong ca (`0` = không giới hạn).
  - `Thời gian ca (phút)`: Thời gian tính từ lúc shipper rời Kho `(0, 0)` đến khi quay về Kho (`0` = không giới hạn).
  - `Hệ số đường (1.0–3.0)`: Nhân khoảng cách chim bay Euclid để ước lượng đường phố thực tế (ví dụ `1.25` hoặc `1.3`).
  - `Đồ thị xung đột`: Chọn `Chính xác (2 chiều)` (xét cả hai thứ tự giao) hoặc `Bảo toàn (v1)` (điều kiện chặt để đối chiếu lý thuyết).

### 2.2. Tab 1 — Điều phối & Bản đồ
- Quan sát lộ trình của từng shipper từ **KHO `(0, 0)`**: nét liền có **mũi tên chỉ chiều đi**, vòng tròn nhỏ ghi **số thứ tự điểm dừng `1, 2, 3...`**, nét đứt chỉ chiều về Kho.
- **Tương tác trực tiếp trên bản đồ:**
  - **Nhấp vào một đỉnh đơn hàng** để mở hộp thoại chỉnh sửa thông tin đơn.
  - **Nhấn giữ và kéo đỉnh đơn hàng** trên bản đồ để thay đổi tọa độ `(X, Y)` trực quan — C++ sẽ tự động tính lại lịch.
  - Bật **`+ Chấm điểm thêm đơn`** rồi nhấp vào khoảng trống trên bản đồ để tạo đơn mới ngay tại vị trí đó.
- **Mô phỏng xe chạy theo thời gian thực:** Bấm **`▶ Mô phỏng xe chạy`** (hoặc kéo thanh trượt thời gian) để quan sát các shipper `S1, S2...` di chuyển, chờ mở cửa sổ và giao hàng theo từng phút trong ngày.

### 2.3. Tab 2 — Đồ thị Xung đột & Tô màu Đồ thị
- Chọn thuật toán trong 4 thuật toán (`Greedy`, `Welsh-Powell`, `DSATUR`, `Backtracking`).
- Bấm **`▶ Tự chạy`** hoặc **`◀ Bước trước` / `Bước tiếp ▶`** để xem C++ chọn đỉnh nào tiếp theo, bậc `deg(v)`, độ bão hòa `sat(v)`, tập màu bị cấm ở các đỉnh kề và màu được gán.
- **Nhấp vào bất kỳ đỉnh nào trên đồ thị xung đột** để làm sáng toàn bộ các cạnh xung đột của nó và đọc giải thích lý do xung đột (quá tải trọng, trùng khung giờ hay vượt thời lượng ca).
- Phía dưới hiển thị đầy đủ **Bảng so sánh 4 thuật toán tô màu** và **Ma trận kề `A[u][v]`** kèm kiểm chứng Định lý Bắt tay `∑deg(v) = 2|E|`.

### 2.4. Tab 3 — Lịch trình & Biểu đồ Gantt
- **Biểu đồ Gantt:** Thể hiện trục thời gian trong ngày của từng shipper (màu xám = di chuyển, màu cam = chờ mở cửa sổ, màu shipper = đang phục vụ giao hàng, khung nét đứt = tổng thời lượng ca).
- **Thẻ lộ trình chi tiết:** Liệt kê từng điểm dừng, khung giờ yêu cầu, giờ đến thực tế, số phút chờ, giờ bắt đầu và giờ hoàn thành.

### 2.5. Tab 4 — Minh họa Lý thuyết Slide (`BFS`, `DFS`, `Prim`, `Kruskal`, `Dijkstra`)
- Chọn thuật toán, **Đỉnh xuất phát**, **Đỉnh đích** (với Dijkstra) và **Mạng lưới đường** (`Mạng đường lân cận k = 4`, `k = 3` hoặc `Đồ thị đầy đủ`).
- **Mô phỏng sự cố / kẹt xe:** Trên sơ đồ của `Dijkstra`, `Prim` hoặc `Kruskal`, **nhấp trực tiếp vào bất kỳ đoạn đường nào** để khóa đoạn đường đó (hiện dấu `✕` màu đỏ) — máy chủ C++ sẽ lập tức tìm đường vòng nhiều chặng hoặc cây khung thay thế!
- Với **Dijkstra**, bảng bên dưới hiển thị đầy đủ **Bảng cập nhật nhãn từng bước `(L(v), trước(v))`** đúng ký hiệu trên slide bài giảng của giảng viên.

### 2.6. Tab 5 — Quản lý Đơn hàng & Định dạng File CSV
- Hỗ trợ tìm kiếm theo mã hoặc tên địa điểm tiếng Việt, lọc đơn chưa phân công, nhập/xuất CSV và lưu/mở phiên làm việc `.json`.
- **Định dạng file CSV chuẩn** (dòng đầu là tiêu đề, thời gian tính bằng số phút từ `00:00`, ví dụ `09:30` = `570`):
  ```csv
  id,ten,x,y,bat_dau,ket_thuc,phuc_vu,khoi_luong
  101,"ĐH Sư Phạm CS1 (Q.5)",-4.8,6.9,574,641,12,10
  102,"Cảng Khánh Hội (Q.4)",6.4,-6.0,687,732,7,6
  ```
