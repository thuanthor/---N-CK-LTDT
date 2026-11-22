# ShipperGraph — Đồ án Lý thuyết Đồ thị & Ứng dụng (HCMUE)

**Đề tài:** *Ứng dụng Tô màu Đồ thị và các Thuật toán Đồ thị để Sắp xếp Lịch trình Giao hàng Có Khung giờ cho Shipper*  
**Kiến trúc:** Backend **C++17 thuần** (100% thuật toán đồ thị & tối ưu lịch) + Giao diện Web tách bạch **HTML5 / CSS3 / JavaScript**.

---

## 1. Cách chạy & Biên dịch nhanh

- **Mở giao diện Web ngay:** Nhấp đúp **`MO-GIAO-DIEN.cmd`** (chương trình tự khởi động máy chủ nội bộ `http://127.0.0.1:8080/` và mở trình duyệt; không cần cài Python, Node.js hay thư viện ngoài).
- **Biên dịch lại khi sửa code C++ trong `src/`:**
  ```powershell
  .\MO-GIAO-DIEN.cmd --build
  ```
- **Chạy bộ 477 kiểm thử tự động:**
  ```powershell
  .\MO-GIAO-DIEN.cmd --test
  ```
- **Đóng gói tự động ra đúng 3 thư mục nộp bài (`Document`, `Source`, `Release`) theo Quy định HCMUE:**
  ```powershell
  .\DONG-GOI-NOP-BAI.cmd
  ```

---

## 2. Cấu trúc Thư mục Rõ ràng (Phục vụ Trình bày & Vấn đáp Q&A)

| Đường dẫn | Vai trò |
|---|---|
| `src/model.h` | Cấu trúc `Order` (lưu đầy đủ tên địa điểm), `Cfg`, `Graph` (Ma trận kề + Danh sách kề), mô phỏng cơ sở `simulate()`, đọc/xuất CSV, bộ mẫu 14 đơn & 18 địa điểm TP.HCM |
| `src/coloring.h` | 4 thuật toán Tô màu đỉnh (`Greedy`, `Welsh-Powell`, `DSATUR`, `Backtracking` nhánh cận), cận dưới `maxClique`, lưu nhật ký từng bước `ColorStep` |
| `src/scheduler.h` | Hậu xử lý lịch thực tế: `repairColoring` (sửa tải/ca), `mergeGroups` (gộp shipper), `relocate` & `bestSequence` (giảm km), hàm kiểm chứng độc lập `validatePlan` |
| `src/theory.h` | Cài đặt chuẩn theo slide TS. Nguyễn Viết Hưng: `BFS`, `DFS`, `Prim`, `Kruskal` (DSU), `Dijkstra` kèm **bảng cập nhật nhãn từng bước** và chế độ mạng đường lân cận `k-NN` / khóa cạnh sự cố |
| `src/http_server.h` | Máy chủ HTTP nội bộ (`127.0.0.1`) viết bằng Winsock2 chuẩn (`-lws2_32`), phục vụ `web/` và REST API `/api/solve`, `/api/presets` |
| `src/self_test.h` | Bộ 477 kiểm thử tích hợp (vét cạn 64 đồ thị 4 đỉnh, từng bước tô màu, bảng nhãn Dijkstra, mạng đường khóa cạnh, kiểm chứng lịch) |
| `src/main.cpp` | Điểm vào chương trình (`main`), xử lý tham số dòng lệnh UTF-8 trên Windows |
| `web/index.html` | Bố cục HTML5 gồm 5 phân hệ chính ngay trên thanh điều hướng |
| `web/styles.css` | Bảng định kiểu CSS tách riêng, hỗ trợ giao diện Sáng / Tối & Responsive |
| `web/app.js` | Logic giao diện: gọi API C++, vẽ Bản đồ có mũi tên + mô phỏng xe chạy theo phút, chạy từng bước Tô màu & bảng nhãn Dijkstra, quản lý CSV/JSON |
| `Document/` | Hướng dẫn sử dụng, Dàn ý báo cáo Word chuẩn quy định HCMUE và Cẩm nang ôn tập Q&A code |
| `_backup_ban_cu/` | Bản sao lưu nguyên trạng của 2 file gộp cũ trước khi nâng cấp |

---

## 3. Đối chiếu Trực tiếp với Slide Bài giảng Học phần

| Nội dung trong Slide | Tài liệu & Trang | Hàm C++ cài đặt | Minh họa trên Giao diện Web |
|---|---|---|---|
| Đơn đồ thị vô hướng, Ma trận kề, Bậc đỉnh & Định lý bắt tay | `Đồ thị(1).pdf`, tr. 2, 6, 23, 26, 34 | `struct Graph`, `buildConflictGraph()` (`src/model.h`) | Tab **2. Đồ thị Xung đột & Tô màu**: Vẽ đồ thị, hiện ma trận kề `A[u][v]`, kiểm chứng `∑deg(v) = 2|E|` |
| Liên thông, Duyệt rộng BFS & Duyệt sâu DFS tạo rừng khung | `Đồ thị(1).pdf` tr. 46; `Cây_SV2020.pdf` tr. 47–53 | `theory::traverse()` (`src/theory.h`) | Tab **4. Lý thuyết Slide**: Chạy từng bước BFS/DFS từ đỉnh tùy chọn, hiện mức `level` và `parent` |
| Cây khung nhỏ nhất Prim & Kruskal (DSU) | `Cây_SV2020.pdf`, tr. 8–20 | `theory::prim()`, `theory::kruskal()` (`src/theory.h`) | Tab **4. Lý thuyết Slide**: Chạy từng bước kết nạp cạnh nhẹ nhất trên mạng đường |
| Đường đi ngắn nhất Dijkstra & Bảng cập nhật nhãn `[L(v), trước(v)]` | `Đồ thị(1).pdf`, tr. 70–73 | `theory::dijkstra()` (`src/theory.h`) | Tab **4. Lý thuyết Slide**: Hiện đầy đủ bảng nhãn từng vòng lặp chốt đỉnh và cho phép nhấp khóa cạnh đường kẹt xe |
| Phần mở rộng đề tài: Tô màu đồ thị & Lập lịch có khung giờ | Mở rộng thực tiễn | `coloring.h` & `scheduler.h` | Tab **1, 2, 3**: Step-by-step `Greedy` / `Welsh-Powell` / `DSATUR`, sửa lịch, Gantt và mô phỏng xe chạy |
