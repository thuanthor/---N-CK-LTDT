// =============================================================================
// FILE: src/Graph.cpp
// MÔ TẢ: Cài đặt các hàm xử lý Ma trận kề G[MAX][MAX] giống bài Thực hành 1.
// =============================================================================

#include "Graph.h"

Graph::Graph() {
    khoiTao(0);
}

void Graph::khoiTao(int n) {
    if (n < 0) n = 0;
    if (n > MAX) n = MAX;
    soDinh = n;

    for (int i = 0; i < soDinh; i++) {
        for (int j = 0; j < soDinh; j++) {
            G[i][j] = 0;
        }
    }
}

void Graph::themCanh(int u, int v) {
    // Đơn đồ thị vô hướng: không nối khuyên (u == v) và đối xứng G[u][v] = G[v][u] = 1
    if (u < 0 || v < 0 || u >= soDinh || v >= soDinh || u == v) {
        return;
    }
    G[u][v] = 1;
    G[v][u] = 1;
}

int Graph::layGiaTri(int u, int v) const {
    if (u < 0 || v < 0 || u >= soDinh || v >= soDinh) {
        return 0;
    }
    return G[u][v];
}

int Graph::laySoDinh() const {
    return soDinh;
}

// Kiểm tra không có khuyên: mọi phần tử trên đường chéo chính bằng 0
bool Graph::kiemTraKhongKhuyen() const {
    for (int i = 0; i < soDinh; i++) {
        if (G[i][i] != 0) {
            return false;
        }
    }
    return true;
}

// Kiểm tra đồ thị vô hướng: ma trận đối xứng qua đường chéo chính
bool Graph::kiemTraVoHuong() const {
    for (int i = 0; i < soDinh; i++) {
        for (int j = 0; j < soDinh; j++) {
            if (G[i][j] != G[j][i]) {
                return false;
            }
        }
    }
    return true;
}

// Bậc của đỉnh i trong đơn đồ thị vô hướng = tổng các phần tử trên hàng i
int Graph::tinhBac(int i) const {
    if (i < 0 || i >= soDinh) {
        return 0;
    }
    int bac = 0;
    for (int j = 0; j < soDinh; j++) {
        if (G[i][j] != 0) {
            bac++;
        }
    }
    return bac;
}

// Số cạnh của đơn đồ thị vô hướng = tổng số 1 trong ma trận chia 2
int Graph::tinhSoCanh() const {
    int demSo1 = 0;
    for (int i = 0; i < soDinh; i++) {
        for (int j = 0; j < soDinh; j++) {
            if (G[i][j] == 1) {
                demSo1++;
            }
        }
    }
    return demSo1 / 2;
}

// Đỉnh bậc 0 là đỉnh cô lập
bool Graph::laDinhCoLap(int i) const {
    return tinhBac(i) == 0;
}

// Đỉnh bậc 1 là đỉnh treo
bool Graph::laDinhTreo(int i) const {
    return tinhBac(i) == 1;
}
