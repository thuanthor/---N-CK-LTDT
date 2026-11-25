#ifndef GRAPH_H
#define GRAPH_H

// =============================================================================
// FILE: src/Graph.h
// MÔ TẢ: Lớp Graph biểu diễn Đơn đồ thị vô hướng bằng Ma trận kề G[MAX][MAX].
//        Áp dụng đúng kiến thức Thực hành 1 trong vở Lý thuyết Đồ thị:
//        - Kiểm tra không có khuyên (G[i][i] == 0) và đối xứng (G[i][j] == G[j][i])
//        - Tính bậc của đỉnh (tổng hàng), đếm số cạnh (tổng số 1 chia 2)
//        - Xác định đỉnh cô lập (bậc = 0) và đỉnh treo (bậc = 1)
// =============================================================================

#include <string>

#define MAX 100

// Thông tin 1 đơn hàng (tương ứng 1 đỉnh trên đồ thị xung đột)
struct DonHang {
    std::string maDon;      // Mã đơn hàng (VD: "DH01")
    std::string diaChi;     // Địa chỉ giao hàng
    double x;               // Tọa độ X (km tính từ Kho)
    double y;               // Tọa độ Y (km tính từ Kho)
    int gioMo;              // Giờ bắt đầu nhận hàng (phút tính từ 00:00)
    int gioDong;            // Giờ kết thúc nhận hàng (phút tính từ 00:00)
    int phutGiao;           // Thời gian đứng giao hàng (phút)
    int khoiLuong;          // Khối lượng hàng (kg)
};

// Lưu 1 cạnh xung đột giữa đơn u và đơn v
struct CanhXungDot {
    int u;
    int v;
    std::string lyDo;
};

// Lưu 1 bước tô màu để minh họa từng bước (Step-by-Step) trên giao diện
struct BuocToMau {
    int buoc;
    int dinh;
    int mau;
    int bac;
    std::string giaiThich;
};

// Kết quả thuật toán Tô màu đồ thị Welsh-Powell
struct KetQuaToMau {
    int soMau;                  // Tổng số màu = Số lượng Shipper (Xe) cần dùng
    int mauCuaDinh[MAX];        // mauCuaDinh[i] = chỉ số màu của đỉnh i (0, 1, 2...)
    BuocToMau cacBuoc[MAX];     // Mảng lưu thứ tự từng bước tô
    int soBuoc;                 // Tổng số bước đã ghi nhận
};

// Thông tin phân bổ đơn và quãng đường của 1 Xe (Shipper)
struct LoTrinhXe {
    int maXe;                   // Xe 1, Xe 2, ...
    int dsDon[MAX];             // Mảng lưu chỉ số các đơn hàng của xe này
    int soDon;                  // Số lượng đơn hàng trên xe
    int tongKhoiLuong;          // Tổng số kg hàng trên xe
    double tongQuangDuong;      // Tổng độ dài đường đi từ Ma trận khoảng cách D[i][j]
};

// Lớp Đồ thị vô hướng (OOP cơ bản)
class Graph {
private:
    int soDinh;
    int G[MAX][MAX];            // Ma trận kề 0 / 1

public:
    Graph();

    // Khởi tạo đồ thị rỗng gồm n đỉnh (tất cả G[i][j] = 0)
    void khoiTao(int n);

    // Thêm cạnh vô hướng nối hai đỉnh u và v
    void themCanh(int u, int v);

    // Lấy giá trị G[u][v] trong ma trận kề (1 là có cạnh, 0 là không có cạnh)
    int layGiaTri(int u, int v) const;

    // Lấy số đỉnh của đồ thị
    int laySoDinh() const;

    // Kiểm tra đồ thị không có khuyên: đường chéo chính G[i][i] == 0
    bool kiemTraKhongKhuyen() const;

    // Kiểm tra đồ thị vô hướng: ma trận đối xứng G[i][j] == G[j][i]
    bool kiemTraVoHuong() const;

    // Tính bậc deg(i) của đỉnh i: cộng tổng hàng i
    int tinhBac(int i) const;

    // Tính tổng số cạnh của đơn đồ thị vô hướng: đếm số 1 trong ma trận rồi chia 2
    int tinhSoCanh() const;

    // Kiểm tra đỉnh đặc biệt (Thực hành 1): Đỉnh cô lập (bậc 0) & Đỉnh treo (bậc 1)
    bool laDinhCoLap(int i) const;
    bool laDinhTreo(int i) const;
};

#endif
