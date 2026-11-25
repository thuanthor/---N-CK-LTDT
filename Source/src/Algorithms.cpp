// =============================================================================
// FILE: src/Algorithms.cpp
// MÔ TẢ: Cài đặt bằng mảng tĩnh, vòng lặp for và hàm visit() đệ quy cơ bản.
//        Hoàn toàn không dùng cú pháp hay thuật toán ngoài chương trình học.
// =============================================================================

#include "Algorithms.h"
#include <cmath>

using namespace std;

// Đổi số phút tính từ 00:00 sang chuỗi "HH:MM" (VD: 495 -> "08:15")
string doiPhutSangGio(int phut) {
    int gio = phut / 60;
    int phutLe = phut % 60;
    string sGio = (gio < 10 ? "0" : "") + to_string(gio);
    string sPhut = (phutLe < 10 ? "0" : "") + to_string(phutLe);
    return sGio + ":" + sPhut;
}

// Tính khoảng cách giữa 2 điểm theo công thức Pytago
double tinhKhoangCachHaiDiem(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return sqrt(dx * dx + dy * dy) * HE_SO_DUONG;
}

int tinhPhutDiChuyen(double km) {
    return (int)ceil(km / TOC_DO_KM_PHUT);
}

// -----------------------------------------------------------------------------
// 1. LẬP MA TRẬN KHOẢNG CÁCH (TRỌNG SỐ) D[i][j]
//    Đỉnh 0 là Kho trung tâm (0, 0); các đỉnh 1..n là đơn hàng thứ 0..n-1
//    Khi i == j thì D[i][j] = 0; khi i != j thì D[i][j] = khoảng cách giữa i và j
// -----------------------------------------------------------------------------
void lapMaTranKhoangCach(const DonHang dsDon[], int n, double D[MAX][MAX]) {
    int tongDinh = n + 1;
    for (int i = 0; i < tongDinh; i++) {
        for (int j = 0; j < tongDinh; j++) {
            if (i == j) {
                D[i][j] = 0.0;
            } else {
                double x1 = (i == 0) ? 0.0 : dsDon[i - 1].x;
                double y1 = (i == 0) ? 0.0 : dsDon[i - 1].y;
                double x2 = (j == 0) ? 0.0 : dsDon[j - 1].x;
                double y2 = (j == 0) ? 0.0 : dsDon[j - 1].y;
                D[i][j] = tinhKhoangCachHaiDiem(x1, y1, x2, y2);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 2. XÉT TÍNH LIÊN THÔNG CỦA ĐỒ THỊ (Đúng 4 bước trên Slide của thầy Hưng)
//    visited[i] = 0 (chưa duyệt), visited[i] = k (thuộc miền liên thông thứ k)
// -----------------------------------------------------------------------------
void visit(const Graph& g, int i, int nhan, int visited[]) {
    visited[i] = nhan;
    int n = g.laySoDinh();
    for (int j = 0; j < n; j++) {
        if (g.layGiaTri(i, j) != 0 && visited[j] == 0) {
            visit(g, j, nhan, visited);
        }
    }
}

int xetLienThong(const Graph& g, int visited[]) {
    int n = g.laySoDinh();

    // Bước 1: Đánh dấu tất cả các đỉnh là chưa được duyệt (0)
    for (int i = 0; i < n; i++) {
        visited[i] = 0;
    }
    int soMienLienThong = 0;

    // Bước 2 & Bước 3: Tìm đỉnh chưa duyệt, tăng biến đếm và gọi visit()
    for (int i = 0; i < n; i++) {
        if (visited[i] == 0) {
            soMienLienThong++;
            visit(g, i, soMienLienThong, visited);
        }
    }
    return soMienLienThong;
}

// -----------------------------------------------------------------------------
// 3. KIỂM TRA XUNG ĐỘT & XÂY DỰNG ĐỒ THỊ XUNG ĐỘT G = (V, E)
// -----------------------------------------------------------------------------
static bool kiemTraDiKip(const DonHang& truoc, const DonHang& sau) {
    if (truoc.gioMo > truoc.gioDong) {
        return false;
    }
    int xongDonTruoc = truoc.gioMo + truoc.phutGiao;
    double km = tinhKhoangCachHaiDiem(truoc.x, truoc.y, sau.x, sau.y);
    int thoiDiemDenSau = xongDonTruoc + tinhPhutDiChuyen(km);

    // Kịp nếu đến điểm sau trước hoặc đúng giờ đóng cửa
    return thoiDiemDenSau <= sau.gioDong;
}

bool kiemTraXungDot(const DonHang& a, const DonHang& b, int taiToiDa, string& lyDo) {
    // Xung đột 1: Tổng khối lượng 2 đơn vượt quá tải trọng tối đa của 1 xe
    if (taiToiDa > 0 && (a.khoiLuong + b.khoiLuong) > taiToiDa) {
        lyDo = "Quá tải trọng xe (" + to_string(a.khoiLuong + b.khoiLuong) +
               "kg > " + to_string(taiToiDa) + "kg)";
        return true;
    }

    // Xung đột 2: Giao a -> b không kịp giờ VÀ giao b -> a cũng không kịp giờ
    bool kipAB = kiemTraDiKip(a, b);
    bool kipBA = kiemTraDiKip(b, a);
    if (!kipAB && !kipBA) {
        int phutDi = tinhPhutDiChuyen(tinhKhoangCachHaiDiem(a.x, a.y, b.x, b.y));
        lyDo = "Trùng khung giờ + cách " + to_string(phutDi) + " phút chạy xe";
        return true;
    }

    return false;
}

void xayDungDoThiXungDot(const DonHang dsDon[],
                         int n,
                         int taiToiDa,
                         Graph& g,
                         CanhXungDot dsCanh[],
                         int& soCanh) {
    g.khoiTao(n);
    soCanh = 0;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            string lyDo;
            if (kiemTraXungDot(dsDon[i], dsDon[j], taiToiDa, lyDo)) {
                g.themCanh(i, j);
                dsCanh[soCanh].u = i;
                dsCanh[soCanh].v = j;
                dsCanh[soCanh].lyDo = lyDo;
                soCanh++;
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 4. THUẬT TOÁN TÔ MÀU ĐỒ THỊ WELSH-POWELL (3 bước chuẩn)
//    - Bước 1: Tính bậc của từng đỉnh
//    - Bước 2: Sắp xếp các đỉnh theo thứ tự bậc giảm dần
//    - Bước 3: Tô màu mới cho đỉnh đầu tiên chưa tô và các đỉnh không kề với nó
// -----------------------------------------------------------------------------
KetQuaToMau toMauWelshPowell(const Graph& g, const DonHang dsDon[], int taiToiDa) {
    int n = g.laySoDinh();
    KetQuaToMau kq;
    kq.soMau = 0;
    kq.soBuoc = 0;

    for (int i = 0; i < n; i++) {
        kq.mauCuaDinh[i] = -1; // -1 nghĩa là đỉnh chưa được tô màu
    }

    if (n == 0) {
        return kq;
    }

    // Bước 1 & Bước 2: Khởi tạo mảng thứ tự đỉnh 0..n-1 rồi sắp xếp bậc giảm dần
    int thuTu[MAX];
    for (int i = 0; i < n; i++) {
        thuTu[i] = i;
    }

    // Đổi chỗ trực tiếp (giữ nguyên thứ tự đỉnh nhỏ đứng trước khi bằng bậc)
    for (int i = 0; i < n - 1; i++) {
        int viTriMax = i;
        for (int j = i + 1; j < n; j++) {
            if (g.tinhBac(thuTu[j]) > g.tinhBac(thuTu[viTriMax])) {
                viTriMax = j;
            }
        }
        int tam = thuTu[i];
        thuTu[i] = thuTu[viTriMax];
        thuTu[viTriMax] = tam;
    }

    // Bước 3: Lần lượt mở từng màu (Màu 0 = Xe 1, Màu 1 = Xe 2...)
    int mauHienTai = 0;
    int soDinhDaTo = 0;

    while (soDinhDaTo < n) {
        int nhomCungMau[MAX];
        int demTrongNhom = 0;
        int taiHienTai = 0;

        for (int i = 0; i < n; i++) {
            int u = thuTu[i];
            if (kq.mauCuaDinh[u] != -1) {
                continue; // Đỉnh này đã được tô màu ở lượt trước
            }

            // Kiểm tra u có nối cạnh xung đột với đỉnh nào đã mang màu này chưa
            bool biKe = false;
            for (int k = 0; k < demTrongNhom; k++) {
                int v = nhomCungMau[k];
                if (g.layGiaTri(u, v) == 1) {
                    biKe = true;
                    break;
                }
            }

            // Kiểm tra thêm đơn u vào xe này có vượt tải trọng tối đa không
            bool vuotTai = (demTrongNhom > 0 && taiToiDa > 0 &&
                            taiHienTai + dsDon[u].khoiLuong > taiToiDa);

            if (!biKe && !vuotTai) {
                kq.mauCuaDinh[u] = mauHienTai;
                nhomCungMau[demTrongNhom] = u;
                demTrongNhom++;
                taiHienTai += dsDon[u].khoiLuong;
                soDinhDaTo++;

                // Lưu lại bước tô màu để chiếu Step-by-Step
                BuocToMau& b = kq.cacBuoc[kq.soBuoc];
                b.buoc = kq.soBuoc + 1;
                b.dinh = u;
                b.mau = mauHienTai;
                b.bac = g.tinhBac(u);
                b.giaiThich = "Xét " + dsDon[u].maDon + " (bậc deg = " +
                              to_string(b.bac) + "): không kề đỉnh cùng màu, tổng tải " +
                              to_string(taiHienTai) + "kg → Tô Màu " + to_string(mauHienTai + 1) +
                              " (Xe " + to_string(mauHienTai + 1) + ")";
                kq.soBuoc++;
            }
        }
        mauHienTai++;
    }

    kq.soMau = mauHienTai;
    return kq;
}

// Gom các đơn cùng màu vào từng Xe, sắp theo giờ mở cửa và cộng độ dài đường đi từ D[i][j]
int taoDanhSachXe(const DonHang dsDon[],
                  int n,
                  const double D[MAX][MAX],
                  const KetQuaToMau& kqToMau,
                  LoTrinhXe dsXe[]) {
    for (int mau = 0; mau < kqToMau.soMau; mau++) {
        LoTrinhXe& xe = dsXe[mau];
        xe.maXe = mau + 1;
        xe.soDon = 0;
        xe.tongKhoiLuong = 0;
        xe.tongQuangDuong = 0.0;

        for (int i = 0; i < n; i++) {
            if (kqToMau.mauCuaDinh[i] == mau) {
                xe.dsDon[xe.soDon] = i;
                xe.soDon++;
                xe.tongKhoiLuong += dsDon[i].khoiLuong;
            }
        }

        // Sắp xếp các đơn trong xe theo giờ mở cửa tăng dần để đi giao lần lượt
        for (int i = 0; i < xe.soDon - 1; i++) {
            for (int j = i + 1; j < xe.soDon; j++) {
                if (dsDon[xe.dsDon[j]].gioMo < dsDon[xe.dsDon[i]].gioMo) {
                    int tam = xe.dsDon[i];
                    xe.dsDon[i] = xe.dsDon[j];
                    xe.dsDon[j] = tam;
                }
            }
        }

        // Cộng độ dài đường đi từ Ma trận khoảng cách D: Kho (0) -> các đơn -> về Kho (0)
        // (Lưu ý: đơn có chỉ số idx trong dsDon tương ứng đỉnh idx + 1 trong ma trận D)
        int dinhHienTai = 0;
        for (int i = 0; i < xe.soDon; i++) {
            int dinhTiepTheo = xe.dsDon[i] + 1;
            xe.tongQuangDuong += D[dinhHienTai][dinhTiepTheo];
            dinhHienTai = dinhTiepTheo;
        }
        if (xe.soDon > 0) {
            xe.tongQuangDuong += D[dinhHienTai][0];
        }
    }

    return kqToMau.soMau;
}
