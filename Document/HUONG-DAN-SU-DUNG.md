# HƯỚNG DẪN SỬ DỤNG CHƯƠNG TRÌNH SHIPPERCOLOR

*(Tài liệu thuộc thư mục `Document/` nộp kèm đồ án học phần Lý thuyết Đồ thị và Ứng dụng — HCMUE)*

---

## 1. Khởi chạy nhanh (Không cần cài đặt thư viện)

1. Trong thư mục `Release/`: nhấp đúp **`CHAY-CHUONG-TRINH.cmd`** (hoặc trong `Source/`: nhấp đúp **`MO-GIAO-DIEN.cmd`**).
2. Chương trình C++ (`parcel.exe`) tự động đọc dữ liệu từ thư mục `input/`, kiểm tra đơn đồ thị vô hướng, xét thành phần liên thông, chạy thuật toán Tô màu **Welsh–Powell**, xuất kết quả Excel vào `output/ket_qua_phan_bo.csv`, cập nhật `web/du_lieu.js` và mở giao diện trực quan trên trình duyệt Web.
3. Nếu muốn thao tác trên cửa sổ dòng lệnh C++ truyền thống (`cin` / `cout`, in Ma trận kề `G[i][j]`), nhấp đúp **`CHAY-MENU-CONSOLE.cmd`**.

---

## 2. Quy trình 5 Bước Demo Trực quan trên Giao diện Web

1. **Bước 1 — Chọn Bộ dữ liệu hoặc Tải file CSV (`📂 1. Tải file CSV`):**
   - **Bộ 1 (10 đơn giờ cao điểm TP.HCM):** Minh họa phân bổ xe thực tế có khung giờ và giới hạn tải trọng (`25 kg`).
   - **Bộ 2 (8 đơn hàng minh họa lý thuyết):** Minh họa trực quan **Đỉnh cô lập (`deg = 0`)**, **Đỉnh treo (`deg = 1`)** và **Số miền liên thông**.
   - Hoặc bấm **`📂 1. Tải file CSV`** để nạp danh sách đơn hàng bất kỳ từ máy tính.
2. **Bước 2 — Dựng Đồ thị Xung đột (`🕸️ 2. Dựng Đồ thị`):**
   - Biểu diễn mỗi đơn hàng là 1 đỉnh và nối cạnh giữa các cặp đơn xung đột (trùng giờ di chuyển hoặc vượt tải xe).
   - Di chuột vào từng đỉnh để xem địa chỉ, khung giờ, khối lượng, bậc `deg`, miền liên thông và khoảng cách tới Kho.
3. **Bước 3 — Tô màu Welsh–Powell (`🎨 3. Tô màu`):**
   - Tô màu các đỉnh theo thứ tự bậc giảm dần sao cho hai đỉnh kề nhau khác màu và tổng khối lượng mỗi xe không vượt tải tối đa.
   - Chuyển sang **`Bản đồ Lộ trình từ Kho`** ở góc phải khung vẽ để xem tuyến đi của từng xe.
4. **Bước 4 — Chạy từng bước (`⏯️ 4. Chạy từng bước`):**
   - Nhấn **`Bước tiếp (NEXT) ▶`** hoặc **`▶ Tự chạy`** để quan sát đỉnh đang xét sáng viền vàng kèm lời giải thích tiếng Việt.
5. **Bước 5 — Xuất kết quả Excel (`📥 5. Xuất Excel (.csv)`):**
   - Tải về file `.csv` chuẩn mã tiếng Việt UTF-8 mở trực tiếp bằng Microsoft Excel gồm *Bảng tổng hợp phân bổ xe* và *Danh sách đơn hàng chi tiết*.
