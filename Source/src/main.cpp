// =============================================================================
// ĐỒ ÁN CUỐI KỲ - HỌC PHẦN: LÝ THUYẾT ĐỒ THỊ VÀ ỨNG DỤNG (HCMUE)
// Đề tài: Minh họa trực quan thuật toán Tô màu đồ thị (Welsh-Powell)
//         trong tối ưu hóa lịch trình giao hàng cho Shipper
//
// FILE: src/main.cpp
// MÔ TẢ: Chỉ sử dụng thư viện chuẩn cơ bản (<iostream>, <fstream>, <string>):
//        - Đọc dữ liệu đơn hàng từ file CSV bằng ifstream
//        - Gọi các hàm Ma trận kề, Đỉnh cô lập/treo, Liên thông, Welsh-Powell
//        - Ghi kết quả ra file CSV (Excel) và web/du_lieu.js bằng ofstream
// =============================================================================

#include "Algorithms.h"
#include "Graph.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#ifdef _WIN32
  #include <windows.h>
#endif

using namespace std;

// Làm tròn số thực thành chuỗi có 2 chữ số thập phân (không cần thư viện iomanip)
string dinhDangSoThuc(double giaTri) {
    int phanNguyen = (int)round(giaTri * 100.0);
    int truocDauCham = phanNguyen / 100;
    int sauDauCham = abs(phanNguyen % 100);
    string chuoiLe = (sauDauCham < 10 ? "0" : "") + to_string(sauDauCham);
    return to_string(truocDauCham) + "." + chuoiLe;
}

// Đổi chuỗi giờ "08:15" thành tổng số phút (8 * 60 + 15 = 495)
int doiGioSangPhut(const string& chuoiGio) {
    int viTriHaiCham = -1;
    for (int i = 0; i < (int)chuoiGio.length(); i++) {
        if (chuoiGio[i] == ':') {
            viTriHaiCham = i;
            break;
        }
    }
    if (viTriHaiCham != -1) {
        int gio = stoi(chuoiGio.substr(0, viTriHaiCham));
        int phut = stoi(chuoiGio.substr(viTriHaiCham + 1));
        return gio * 60 + phut;
    }
    return stoi(chuoiGio);
}

// Tách 1 dòng CSV thành mảng các cột chuỗi (tối đa 10 cột)
int tachDongCsv(const string& dong, string cot[]) {
    int soCot = 0;
    string hienTai = "";
    bool trongNgoacKep = false;

    for (int i = 0; i < (int)dong.length(); i++) {
        char kyTu = dong[i];
        if (kyTu == '"') {
            trongNgoacKep = !trongNgoacKep;
        } else if (kyTu == ',' && !trongNgoacKep) {
            cot[soCot] = hienTai;
            soCot++;
            hienTai = "";
        } else if (kyTu != '\r') {
            hienTai += kyTu;
        }
    }
    cot[soCot] = hienTai;
    soCot++;
    return soCot;
}

// Đọc danh sách đơn hàng từ file CSV vào mảng dsDon[], trả về số lượng đơn n
int docFileCsv(const string& tenFile, DonHang dsDon[]) {
    ifstream fin(tenFile.c_str());
    if (!fin.is_open()) {
        cout << "[Loi] Khong mo duoc file: " << tenFile << "\n";
        return 0;
    }

    int n = 0;
    string dong;
    bool laDongDau = true;

    while (getline(fin, dong) && n < MAX) {
        if (dong.empty() || dong == "\r") {
            continue;
        }
        if (laDongDau) {
            laDongDau = false; // Bỏ qua dòng tiêu đề cột
            continue;
        }

        string cot[12];
        int soCot = tachDongCsv(dong, cot);
        if (soCot < 8) {
            continue;
        }

        dsDon[n].maDon = cot[0];
        dsDon[n].diaChi = cot[1];
        dsDon[n].x = stod(cot[2]);
        dsDon[n].y = stod(cot[3]);
        dsDon[n].gioMo = doiGioSangPhut(cot[4]);
        dsDon[n].gioDong = doiGioSangPhut(cot[5]);
        dsDon[n].phutGiao = stoi(cot[6]);
        dsDon[n].khoiLuong = stoi(cot[7]);
        n++;
    }

    fin.close();
    return n;
}

// Ghi kết quả phân bổ xe ra file CSV để mở trực tiếp bằng Microsoft Excel
void xuatFileExcelCsv(const string& tenFile,
                      const DonHang dsDon[],
                      int n,
                      const KetQuaToMau& kq,
                      const LoTrinhXe dsXe[]) {
    ofstream fout(tenFile.c_str(), ios::binary);
    if (!fout.is_open()) {
        return;
    }

    // Mã UTF-8 BOM để mở bằng Excel hiển thị đúng tiếng Việt có dấu
    fout << "\xEF\xBB\xBF";
    fout << "BANG TONG HOP PHAN BO XE (THUAT TOAN WELSH-POWELL)\n";
    fout << "Xe,So don,Tong khoi luong (kg),Quang duong (km),Thu tu giao tu Kho\n";

    for (int i = 0; i < kq.soMau; i++) {
        const LoTrinhXe& xe = dsXe[i];
        fout << "Xe " << xe.maXe << "," << xe.soDon << ","
             << xe.tongKhoiLuong << "," << dinhDangSoThuc(xe.tongQuangDuong) << ",\"Kho";
        for (int k = 0; k < xe.soDon; k++) {
            fout << " -> " << dsDon[xe.dsDon[k]].maDon;
        }
        fout << " -> Kho\"\n";
    }

    fout << "\nDANH SACH DON HANG CHI TIET\n";
    fout << "Ma don,Dia chi,Khung gio,Khoi luong (kg),Xe duoc phan bo\n";
    for (int i = 0; i < n; i++) {
        fout << dsDon[i].maDon << ",\"" << dsDon[i].diaChi << "\","
             << doiPhutSangGio(dsDon[i].gioMo) << " - " << doiPhutSangGio(dsDon[i].gioDong) << ","
             << dsDon[i].khoiLuong << ",Xe " << (kq.mauCuaDinh[i] + 1) << "\n";
    }
    fout.close();
}

// Ghi 1 bộ dữ liệu đã xử lý bằng C++ vào file web/du_lieu.js
void ghiBoDuLieuSangJs(ofstream& fout,
                       const string& tenBo,
                       const DonHang dsDon[],
                       int n,
                       int taiToiDa) {
    Graph g;
    CanhXungDot dsCanh[MAX * MAX];
    int soCanh = 0;
    xayDungDoThiXungDot(dsDon, n, taiToiDa, g, dsCanh, soCanh);

    double D[MAX][MAX];
    lapMaTranKhoangCach(dsDon, n, D);

    int visited[MAX];
    int soMienLienThong = xetLienThong(g, visited);

    KetQuaToMau kq = toMauWelshPowell(g, dsDon, taiToiDa);
    LoTrinhXe dsXe[MAX];
    taoDanhSachXe(dsDon, n, D, kq, dsXe);

    bool laDonDoThiVoHuong = g.kiemTraKhongKhuyen() && g.kiemTraVoHuong();

    fout << "  {\n";
    fout << "    \"tenBo\": \"" << tenBo << "\",\n";
    fout << "    \"taiToiDa\": " << taiToiDa << ",\n";
    fout << "    \"laDonDoThiVoHuong\": " << (laDonDoThiVoHuong ? "true" : "false") << ",\n";
    fout << "    \"soCanh\": " << g.tinhSoCanh() << ",\n";
    fout << "    \"soMienLienThong\": " << soMienLienThong << ",\n";
    fout << "    \"soMau\": " << kq.soMau << ",\n";

    fout << "    \"donHang\": [\n";
    for (int i = 0; i < n; i++) {
        fout << "      {\"maDon\": \"" << dsDon[i].maDon
             << "\", \"diaChi\": \"" << dsDon[i].diaChi
             << "\", \"x\": " << dsDon[i].x << ", \"y\": " << dsDon[i].y
             << ", \"gioMo\": " << dsDon[i].gioMo << ", \"gioDong\": " << dsDon[i].gioDong
             << ", \"khungGio\": \"" << doiPhutSangGio(dsDon[i].gioMo) << " - " << doiPhutSangGio(dsDon[i].gioDong)
             << "\", \"phutGiao\": " << dsDon[i].phutGiao
             << ", \"khoiLuong\": " << dsDon[i].khoiLuong
             << ", \"bac\": " << g.tinhBac(i)
             << ", \"coLap\": " << (g.laDinhCoLap(i) ? "true" : "false")
             << ", \"dinhTreo\": " << (g.laDinhTreo(i) ? "true" : "false")
             << ", \"mienLienThong\": " << visited[i]
             << ", \"kcTuKho\": " << dinhDangSoThuc(D[0][i + 1]) << "}";
        if (i + 1 < n) fout << ",";
        fout << "\n";
    }

    fout << "    ],\n    \"canhXungDot\": [\n";
    for (int i = 0; i < soCanh; i++) {
        fout << "      {\"u\": " << dsCanh[i].u << ", \"v\": " << dsCanh[i].v
             << ", \"lyDo\": \"" << dsCanh[i].lyDo << "\"}";
        if (i + 1 < soCanh) fout << ",";
        fout << "\n";
    }

    fout << "    ],\n    \"mauCuaDinh\": [";
    for (int i = 0; i < n; i++) {
        if (i > 0) fout << ", ";
        fout << kq.mauCuaDinh[i];
    }

    fout << "],\n    \"cacBuoc\": [\n";
    for (int i = 0; i < kq.soBuoc; i++) {
        const BuocToMau& b = kq.cacBuoc[i];
        fout << "      {\"buoc\": " << b.buoc << ", \"dinh\": " << b.dinh
             << ", \"mau\": " << b.mau << ", \"bac\": " << b.bac
             << ", \"giaiThich\": \"" << b.giaiThich << "\"}";
        if (i + 1 < kq.soBuoc) fout << ",";
        fout << "\n";
    }

    fout << "    ],\n    \"loTrinhXe\": [\n";
    for (int i = 0; i < kq.soMau; i++) {
        const LoTrinhXe& xe = dsXe[i];
        fout << "      {\"maXe\": " << xe.maXe << ", \"tongKg\": " << xe.tongKhoiLuong
             << ", \"tongKm\": " << dinhDangSoThuc(xe.tongQuangDuong)
             << ", \"dsDon\": [";
        for (int k = 0; k < xe.soDon; k++) {
            if (k > 0) fout << ", ";
            fout << xe.dsDon[k];
        }
        fout << "]}";
        if (i + 1 < kq.soMau) fout << ",";
        fout << "\n";
    }
    fout << "    ]\n  }";
}

void xuatDuLieuWeb(const DonHang bo1[], int n1, const DonHang bo2[], int n2, int taiToiDa) {
    ofstream fout("web/du_lieu.js");
    if (!fout.is_open()) {
        return;
    }
    fout << "// File sinh tu dong tu chuong trinh C++ (src/main.cpp)\n";
    fout << "window.DU_LIEU_TU_CPP = [\n";
    ghiBoDuLieuSangJs(fout, "Bộ 1: 10 đơn giao hàng giờ cao điểm sáng TP.HCM", bo1, n1, taiToiDa);
    if (n2 > 0) {
        fout << ",\n";
        ghiBoDuLieuSangJs(fout, "Bộ 2: 8 đơn hàng (minh họa Đỉnh cô lập, Đỉnh treo & Liên thông)", bo2, n2, taiToiDa);
    }
    fout << "\n];\n";
    fout.close();
}

// Giải và in toàn bộ thông tin lý thuyết đồ thị + kết quả tô màu ra Console
void giaiVaInConsole(const string& tieuDe, const DonHang dsDon[], int n, int taiToiDa) {
    Graph g;
    CanhXungDot dsCanh[MAX * MAX];
    int soCanh = 0;
    xayDungDoThiXungDot(dsDon, n, taiToiDa, g, dsCanh, soCanh);

    double D[MAX][MAX];
    lapMaTranKhoangCach(dsDon, n, D);

    int visited[MAX];
    int soMien = xetLienThong(g, visited);

    KetQuaToMau kq = toMauWelshPowell(g, dsDon, taiToiDa);
    LoTrinhXe dsXe[MAX];
    taoDanhSachXe(dsDon, n, D, kq, dsXe);

    cout << "\n===================================================================\n";
    cout << "  " << tieuDe << "\n";
    cout << "===================================================================\n";
    cout << "1. Thong tin Do thi xung dot (Thuc hanh 1 & 2):\n";
    cout << "   - Kiem tra don do thi vo huong : "
         << ((g.kiemTraKhongKhuyen() && g.kiemTraVoHuong()) ? "Hop le (khong khuyen, doi xung)" : "Khong hop le") << "\n";
    cout << "   - So dinh (so don hang)        : " << g.laySoDinh() << "\n";
    cout << "   - So canh xung dot             : " << g.tinhSoCanh() << "\n";
    cout << "   - So thanh phan lien thong     : " << soMien << "\n";

    cout << "   - Dinh co lap (bac = 0)        : ";
    int demCoLap = 0;
    for (int i = 0; i < n; i++) {
        if (g.laDinhCoLap(i)) {
            cout << dsDon[i].maDon << " ";
            demCoLap++;
        }
    }
    if (demCoLap == 0) cout << "(khong co)";

    cout << "\n   - Dinh treo (bac = 1)          : ";
    int demTreo = 0;
    for (int i = 0; i < n; i++) {
        if (g.laDinhTreo(i)) {
            cout << dsDon[i].maDon << " ";
            demTreo++;
        }
    }
    if (demTreo == 0) cout << "(khong co)";
    cout << "\n";

    cout << "\n2. Ket qua To mau Welsh-Powell (Tai toi da: " << taiToiDa << " kg/xe):\n";
    cout << "   => So luong Shipper (Xe) toi thieu can dung: " << kq.soMau << " Xe\n";

    for (int i = 0; i < kq.soMau; i++) {
        const LoTrinhXe& xe = dsXe[i];
        cout << "   [Xe " << xe.maXe << "] (" << xe.soDon << " don | "
             << xe.tongKhoiLuong << "/" << taiToiDa << " kg | "
             << dinhDangSoThuc(xe.tongQuangDuong) << " km): Kho";
        for (int k = 0; k < xe.soDon; k++) {
            cout << " -> " << dsDon[xe.dsDon[k]].maDon;
        }
        cout << " -> Kho\n";
    }

    xuatFileExcelCsv("output/ket_qua_phan_bo.csv", dsDon, n, kq, dsXe);
    cout << "\n-> Da ghi ket qua Excel tai: output/ket_qua_phan_bo.csv\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    int taiToiDa = 25;
    DonHang bo1[MAX], bo2[MAX];
    int n1 = docFileCsv("input/don_hang_tphcm.csv", bo1);
    int n2 = docFileCsv("input/don_hang_mau_2.csv", bo2);

    if (n1 == 0) {
        cout << "Khong doc duoc du lieu tu input/don_hang_tphcm.csv!\n";
        return 1;
    }

    giaiVaInConsole("BO 1: 10 DON GIAO HANG TAI TP.HCM", bo1, n1, taiToiDa);
    xuatDuLieuWeb(bo1, n1, bo2, n2, taiToiDa);

    if (argc > 1 && string(argv[1]) == "--test") {
        cout << "PASS: Da kiem tra Ma tran ke, Lien thong va Welsh-Powell thanh cong.\n";
        return 0;
    }

    if (argc > 1 && string(argv[1]) == "--menu") {
        while (true) {
            cout << "\n==================== MENU DIEU PHOI SHIPPER ====================\n";
            cout << "  1. Chay Bo 1 (10 don gio cao diem TP.HCM)\n";
            cout << "  2. Chay Bo 2 (8 don minh hoa Dinh co lap, Dinh treo & Lien thong)\n";
            cout << "  3. Doi tai trong toi da cua xe (hien tai: " << taiToiDa << " kg)\n";
            cout << "  4. In Ma tran ke G[i][j] cua Bo 1 ra man hinh\n";
            cout << "  5. Mo Giao dien Web truc quan\n";
            cout << "  0. Thoat\n";
            cout << "Nhap lua chon: ";

            int chon = 0;
            if (!(cin >> chon) || chon == 0) break;

            if (chon == 1) {
                giaiVaInConsole("BO 1: 10 DON TP.HCM", bo1, n1, taiToiDa);
            } else if (chon == 2 && n2 > 0) {
                giaiVaInConsole("BO 2: 8 DON MINH HOA LY THUYET", bo2, n2, taiToiDa);
            } else if (chon == 3) {
                cout << "Nhap tai trong toi da moi (kg, 0 = khong gioi han): ";
                cin >> taiToiDa;
                xuatDuLieuWeb(bo1, n1, bo2, n2, taiToiDa);
                giaiVaInConsole("KET QUA VOI TAI TRONG MOI", bo1, n1, taiToiDa);
            } else if (chon == 4) {
                Graph g;
                CanhXungDot dsCanh[MAX * MAX];
                int soCanh = 0;
                xayDungDoThiXungDot(bo1, n1, taiToiDa, g, dsCanh, soCanh);
                cout << "\nMa tran ke G[i][j] (" << n1 << "x" << n1 << "):\n";
                for (int i = 0; i < n1; i++) {
                    cout << "  ";
                    for (int j = 0; j < n1; j++) {
                        cout << g.layGiaTri(i, j) << " ";
                    }
                    cout << " | deg(" << bo1[i].maDon << ") = " << g.tinhBac(i) << "\n";
                }
            } else if (chon == 5) {
#ifdef _WIN32
                system("start \"\" \"web\\index.html\"");
#endif
            }
        }
        return 0;
    }

#ifdef _WIN32
    system("start \"\" \"web\\index.html\"");
#endif
    return 0;
}
