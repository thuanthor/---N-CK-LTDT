# ShipperColor C++ — Đồ án Lý thuyết Đồ thị & Ứng dụng (HCMUE)

**Đề tài:** *Minh họa trực quan thuật toán Tô màu đồ thị (Welsh–Powell) trong tối ưu hóa lịch trình giao hàng cho Shipper*  
**Học phần:** Lý thuyết Đồ thị và Ứng dụng — Trường Đại học Sư phạm TP. Hồ Chí Minh (**HCMUE**)

---

## 1. Kiến thức Lý thuyết Đồ thị áp dụng trong Code (100% bám sát bài học trên lớp)

Toàn bộ mã nguồn trong `src/` chỉ sử dụng mảng 2 chiều tĩnh `G[MAX][MAX]`, vòng lặp `for`, hàm đệ quy `visit()` và `ifstream` / `ofstream` cơ bản:

1. **Biểu diễn Ma trận kề & Phân loại đồ thị (`src/Graph.h`, `src/Graph.cpp` — Thực hành 1):**
   - Kiểm tra đồ thị không có khuyên: đường chéo chính `G[i][i] == 0` (`kiemTraKhongKhuyen()`).
   - Kiểm tra đồ thị vô hướng: tính đối xứng `G[i][j] == G[j][i]` (`kiemTraVoHuong()`).
   - Tính bậc đỉnh `tinhBac(i)` (tổng hàng `i`), tính số cạnh `tinhSoCanh()` (đếm số `1` trong ma trận chia `2`).
   - Liệt kê **Đỉnh cô lập (`deg = 0`)** và **Đỉnh treo (`deg = 1`)**.
2. **Xét tính liên thông & Đếm thành phần liên thông (`src/Algorithms.cpp` — Thực hành 2):**
   - Dùng mảng `visited[MAX]` và hàm đệ quy `visit(g, i, nhan, visited)` theo đúng 4 bước trên slide của thầy Nguyễn Viết Hưng.
3. **Ma trận khoảng cách trọng số `D[MAX][MAX]` (`src/Algorithms.cpp`):**
   - $D_{ij} = 0$ khi $i = j$, $D_{ij} = w(v_i, v_j)$ là khoảng cách thực tế (km) giữa Kho (đỉnh `0`) và các đơn hàng (`1..n`).
4. **Phần mở rộng của Đề tài — Tô màu đồ thị `Welsh–Powell` (`toMauWelshPowell()`):**
   - Xếp các đỉnh theo bậc giảm dần, tô cùng một màu (gán chung 1 Xe) cho các đơn không kề nhau và không vượt tải trọng tối đa của xe.

---

## 2. Cách chạy & Biên dịch

- **Chạy C++ và mở ngay Giao diện Web:** Nhấp đúp **`MO-GIAO-DIEN.cmd`**
- **Mở Menu Console C++ (`cin` / `cout`, có mục in Ma trận kề `G[i][j]`):** Nhấp đúp **`CHAY-MENU-CONSOLE.cmd`**
- **Biên dịch thủ công:**
  ```powershell
  g++ -std=c++17 -O2 -Wall -Wextra -static -o parcel.exe src/main.cpp src/Graph.cpp src/Algorithms.cpp
  ```
- **Đóng gói 3 thư mục nộp bài (`Document`, `Source`, `Release`):**
  ```powershell
  .\DONG-GOI-NOP-BAI.cmd
  ```
