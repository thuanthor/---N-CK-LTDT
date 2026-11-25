#ifndef ALGORITHMS_H
#define ALGORITHMS_H

// =============================================================================
// FILE: src/Algorithms.h
// MÔ TẢ: Chỉ sử dụng đúng các kiến thức đã học trong học phần:
//        1. Ma trận khoảng cách (trọng số) D[i][j]
//        2. Xét tính liên thông & đếm thành phần liên thông bằng hàm visit()
//        3. Xây dựng Đơn đồ thị xung đột G = (V, E)
//        4. Thuật toán Tô màu đồ thị Welsh-Powell (phần mở rộng của đề tài)
// =============================================================================

#include "Graph.h"
#include <string>

const double TOC_DO_KM_PHUT = 0.5;   // Vận tốc 30 km/h = 0.5 km/phút
const double HE_SO_DUONG = 1.2;      // Hệ số đường thực tế so với đường thẳng

// Đổi phút sang chuỗi giờ "HH:MM" và tính thời gian di chuyển
std::string doiPhutSangGio(int phut);
double tinhKhoangCachHaiDiem(double x1, double y1, double x2, double y2);
int tinhPhutDiChuyen(double km);

// 1. Lập Ma trận khoảng cách (trọng số) D[0..n][0..n] (0 là Kho, 1..n là các đơn)
void lapMaTranKhoangCach(const DonHang dsDon[], int n, double D[MAX][MAX]);

// 2. Thuật toán tìm số thành phần liên thông (đúng 4 bước trên Slide bài giảng)
void visit(const Graph& g, int i, int nhan, int visited[]);
int xetLienThong(const Graph& g, int visited[]);

// 3. Xây dựng Đơn đồ thị xung đột giữa các đơn hàng
bool kiemTraXungDot(const DonHang& a, const DonHang& b, int taiToiDa, std::string& lyDo);
void xayDungDoThiXungDot(const DonHang dsDon[],
                         int n,
                         int taiToiDa,
                         Graph& g,
                         CanhXungDot dsCanh[],
                         int& soCanh);

// 4. Thuật toán Tô màu đồ thị Welsh-Powell & Lập danh sách xe
KetQuaToMau toMauWelshPowell(const Graph& g, const DonHang dsDon[], int taiToiDa);
int taoDanhSachXe(const DonHang dsDon[],
                  int n,
                  const double D[MAX][MAX],
                  const KetQuaToMau& kqToMau,
                  LoTrinhXe dsXe[]);

#endif
