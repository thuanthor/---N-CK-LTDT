#pragma once
// =============================================================================
// FILE: src/model.h
// MỤC ĐÍCH: Định nghĩa cấu trúc dữ liệu Đơn hàng (Order), Cấu hình ràng buộc
//           (Cfg), Đồ thị xung đột (Graph - Ma trận kề & Danh sách kề), mô phỏng
//           hành trình cơ bản và đọc/ghi CSV.
// =============================================================================

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Hằng số vật lý của bài toán
constexpr double SPEED_KM_PER_MIN = 0.5;   // Tốc độ trung bình: 30 km/giờ = 0.5 km/phút
constexpr double DEPOT_X = 0.0;            // Hoành độ Kho trung tâm (km)
constexpr double DEPOT_Y = 0.0;            // Tung độ Kho trung tâm (km)

// 1. Cấu trúc một đơn hàng (tương ứng 1 đỉnh của đồ thị xung đột)
struct Order {
    int id = 0;            // Mã đơn hàng (duy nhất)
    std::string name;      // Tên địa điểm / khách hàng
    double x = 0.0;        // Tọa độ X so với kho (km)
    double y = 0.0;        // Tọa độ Y so với kho (km)
    int start = 0;         // Giờ mở cửa sổ giao hàng (phút tính từ 00:00)
    int end = 0;           // Giờ đóng cửa sổ bắt đầu giao (phút tính từ 00:00)
    int service = 0;       // Thời gian phục vụ tại điểm giao (phút)
    int weight = 1;        // Khối lượng kiện hàng (kg)
};

// 2. Cấu hình ràng buộc điều phối & tùy chọn mô hình mạng đường lý thuyết
struct Cfg {
    bool exact = true;     // true: kiểm tra xung đột chính xác 2 chiều; false: điều kiện bảo toàn (v1)
    int cap = 0;           // Tải trọng tối đa mỗi shipper (kg), 0 = không giới hạn
    double road = 1.0;     // Hệ số đường thực tế so với đường chim bay (1.0 .. 3.0)
    int shift = 0;         // Thời lượng ca tối đa từ lúc xuất kho đến lúc về kho (phút), 0 = không giới hạn

    // Tùy chọn phục vụ phần Minh họa Lý thuyết (BFS/DFS/Prim/Kruskal/Dijkstra)
    int roadGraphK = 0;    // 0 = đồ thị khoảng cách đầy đủ; k > 0 = mạng đường nối k láng giềng gần nhất
    int theoryRoot = 0;    // Đỉnh xuất phát cho BFS/DFS/Prim/Dijkstra
    std::vector<std::pair<int, int>> blockedEdges; // Các đoạn đường bị khóa (kẹt xe / sự cố)
};

// Thông tin một điểm dừng thực tế trong lịch trình của shipper
struct Stop {
    int order = 0;         // Chỉ số đơn hàng trong mảng orders (0..n-1)
    int arrive = 0;        // Thời điểm đến nơi (phút)
    int begin = 0;         // Thời điểm bắt đầu giao = max(arrive, start)
    int finish = 0;        // Thời điểm giao xong = begin + service
};

// Lộ trình của một shipper ở bước cơ sở (chưa tối ưu thứ tự)
struct Route {
    std::vector<Stop> stops;
    double km = 0.0;
    int returnTime = 0;
    bool feasible = true;
};

// 3. Cấu trúc Đơn đồ thị vô hướng G = (V, E)
//    Lưu đồng thời Ma trận kề (đúng định nghĩa slide Đồ thị(1), trang 23)
//    và Danh sách kề (giúp duyệt láng giềng nhanh O(deg(v))).
struct Graph {
    int n = 0;
    std::vector<std::vector<int>> adj;
    std::vector<std::vector<char>> mat;

    void init(int vertices) {
        n = vertices;
        adj.assign(n, {});
        mat.assign(n, std::vector<char>(n, 0));
    }

    void addEdge(int u, int v) {
        if (u < 0 || v < 0 || u >= n || v >= n || u == v) return;
        if (!mat[u][v]) {
            mat[u][v] = mat[v][u] = 1;
            adj[u].push_back(v);
            adj[v].push_back(u);
        }
    }

    int deg(int v) const { return static_cast<int>(adj[v].size()); }

    int edges() const {
        int sumDeg = 0;
        for (const auto& neighbors : adj) sumDeg += static_cast<int>(neighbors.size());
        return sumDeg / 2; // Định lý bắt tay: tổng bậc = 2 * |E|
    }
};

// ----------------------------- Hàm khoảng cách & thời gian --------------------

inline double dist(double x1, double y1, double x2, double y2, const Cfg& cfg) {
    return std::hypot(x1 - x2, y1 - y2) * cfg.road;
}

inline double depotDist(const Order& o, const Cfg& cfg) {
    return dist(DEPOT_X, DEPOT_Y, o.x, o.y, cfg);
}

inline int depotTime(const Order& o, const Cfg& cfg) {
    return static_cast<int>(std::ceil(depotDist(o, cfg) / SPEED_KM_PER_MIN));
}

inline int travelTime(const Order& a, const Order& b, const Cfg& cfg) {
    return static_cast<int>(std::ceil(dist(a.x, a.y, b.x, b.y, cfg) / SPEED_KM_PER_MIN));
}

inline std::string hhmm(int minutes) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d", minutes / 60, minutes % 60);
    return buf;
}

// Mô phỏng một chuỗi giao hàng cụ thể: Kho -> seq[0] -> ... -> seq[m-1] -> Kho.
// Trả về false nếu có đơn trễ khung giờ hoặc tổng thời gian ca vượt cfg.shift.
inline bool simulate(const std::vector<Order>& orders,
                     const std::vector<int>& seq,
                     const Cfg& cfg,
                     double& outKm,
                     std::vector<Stop>* outStops = nullptr,
                     int* outReturnTime = nullptr) {
    outKm = 0.0;
    if (seq.empty()) return true;

    // Quy ước xuất phát: rời kho sao cho đến đơn đầu tiên đúng giờ mở cửa (hoặc 00:00 nếu cần đi sớm hơn)
    int t = std::max(0, orders[seq[0]].start - depotTime(orders[seq[0]], cfg));
    int departureTime = t;
    double cx = DEPOT_X, cy = DEPOT_Y;

    for (int idx : seq) {
        const Order& ord = orders[idx];
        double d = dist(cx, cy, ord.x, ord.y, cfg);
        int arrive = t + static_cast<int>(std::ceil(d / SPEED_KM_PER_MIN));
        int begin = std::max(arrive, ord.start);
        if (begin > ord.end) return false; // Trễ cửa sổ thời gian
        int finish = begin + ord.service;
        if (outStops) outStops->push_back({idx, arrive, begin, finish});
        outKm += d;
        t = finish;
        cx = ord.x;
        cy = ord.y;
    }

    double back = dist(cx, cy, DEPOT_X, DEPOT_Y, cfg);
    outKm += back;
    int ret = t + static_cast<int>(std::ceil(back / SPEED_KM_PER_MIN));
    if (cfg.shift > 0 && ret - departureTime > cfg.shift) return false; // Vượt thời gian ca
    if (outReturnTime) *outReturnTime = ret;
    return true;
}

// Kiểm tra một đơn lẻ có tự giao được bởi 1 shipper riêng hay không
inline bool isSingleOrderFeasible(const Order& ord, const Cfg& cfg) {
    if (cfg.cap > 0 && ord.weight > cfg.cap) return false;
    int dTime = depotTime(ord, cfg);
    int depart = std::max(0, ord.start - dTime);
    int begin = std::max(depart + dTime, ord.start);
    if (begin > ord.end) return false;
    int ret = begin + ord.service + dTime;
    return cfg.shift <= 0 || (ret - depart <= cfg.shift);
}

// Điều kiện shipper giao đơn a xong rồi giao tiếp đơn b
inline bool canFollow(const Order& a, const Order& b, const Cfg& cfg) {
    if (cfg.exact) {
        // Điều kiện cần: bắt đầu sớm nhất ở a vẫn kịp đến trước giờ đóng cửa của b
        return a.start + a.service + travelTime(a, b, cfg) <= b.end;
    }
    // Điều kiện bảo toàn (v1): kết thúc muộn nhất ở a vẫn kịp trước giờ mở cửa của b
    return a.end + a.service + travelTime(a, b, cfg) <= b.start;
}

// Hai đơn hàng a và b có xung đột (không thể chung 1 shipper) hay không?
inline bool conflict(const Order& a, const Order& b, const Cfg& cfg) {
    if (!cfg.exact) {
        return !canFollow(a, b, cfg) && !canFollow(b, a, cfg);
    }
    // Chế độ chính xác: kiểm tra tổng tải trọng và thử mô phỏng cả hai thứ tự (a -> b) và (b -> a)
    if (cfg.cap > 0 && a.weight + b.weight > cfg.cap) return true;
    std::vector<Order> pairOrders = {a, b};
    double km = 0.0;
    return !simulate(pairOrders, {0, 1}, cfg, km) && !simulate(pairOrders, {1, 0}, cfg, km);
}

// Xây dựng đồ thị xung đột G = (V, E) với độ phức tạp O(n^2)
inline Graph buildConflictGraph(const std::vector<Order>& orders, const Cfg& cfg) {
    Graph g;
    g.init(static_cast<int>(orders.size()));
    for (size_t i = 0; i < orders.size(); ++i) {
        for (size_t j = i + 1; j < orders.size(); ++j) {
            if (conflict(orders[i], orders[j], cfg)) {
                g.addEdge(static_cast<int>(i), static_cast<int>(j));
            }
        }
    }
    return g;
}

// ----------------------------- Dữ liệu mẫu & Sinh ngẫu nhiên ------------------

// Bộ sinh số giả ngẫu nhiên tuyến tính (LCG) đảm bảo kết quả tái lập giống nhau trên mọi máy
struct Rng {
    uint64_t s;
    explicit Rng(uint64_t seed) : s(seed * 6364136223846793005ULL + 1442695040888963407ULL) {}
    uint32_t next() {
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<uint32_t>(s >> 33);
    }
    int range(int a, int b) {
        return a + static_cast<int>(next() % static_cast<uint32_t>(b - a + 1));
    }
};

// Bộ 14 đơn mẫu chuẩn (giữ nguyên thông số kiểm thử tham chiếu, gắn địa danh thực tế TP.HCM)
inline std::vector<Order> sampleOrders() {
    return {
        {1,  "ĐH Sư Phạm CS1 (Q.5)",         -4.8,   6.9,  574,  641, 12, 10},
        {2,  "Cảng Khánh Hội (Q.4)",          6.4,  -6.0,  687,  732,  7,  6},
        {3,  "CV Phần mềm Quang Trung",      -8.0,  11.2,  646,  708, 15,  1},
        {4,  "Bến xe Miền Tây",              -6.0,   5.6,  522,  620, 12, 10},
        {5,  "Sân bay Tân Sơn Nhất",         -1.6,  11.7,  752,  802,  7,  7},
        {6,  "Chợ Bến Thành (Q.1)",           4.3,  -0.4,  547,  646, 13,  5},
        {7,  "Hồ Con Rùa (Q.3)",              1.4,   5.8,  913,  972, 11,  1},
        {8,  "Ngã tư Phú Nhuận",              0.9,   8.8,  817,  897,  5,  8},
        {9,  "ĐH Tôn Đức Thắng (Q.7)",       -1.8,  -7.9,  523,  589,  8, 10},
        {10, "Chợ Xóm Chiếu (Q.4)",          -0.2,  -4.0,  920,  990,  7,  8},
        {11, "Chợ Bà Chiểu (Bình Thạnh)",     2.4,   7.7,  863,  910, 15,  3},
        {12, "Khu Công nghệ cao (Thủ Đức)",  11.5,  10.6,  887,  967,  6,  8},
        {13, "KDC Trung Sơn (Bình Chánh)",   -5.1,  -7.1, 1060, 1114,  5,  9},
        {14, "ĐH Sư Phạm CS2 (Q.3)",         -3.5,  -1.8,  665,  758, 10,  1}
    };
}

// Bộ 18 đơn giao sách & học liệu quanh nội thành TP.HCM (phục vụ trình diễn kịch bản thực tế)
inline std::vector<Order> hcmcOrders() {
    return {
        {201, "ĐH Sư Phạm CS1 - An Dương Vương", -1.2,  -0.8, 480, 540, 10,  8},
        {202, "ĐH Khoa học Tự nhiên (Q.5)",      -1.0,  -0.6, 495, 555,  8,  6},
        {203, "THPT Chuyên Lê Hồng Phong",       -1.1,  -1.0, 510, 575, 10,  7},
        {204, "ĐH Sư Phạm CS2 - Lê Văn Sỹ",      -0.8,   3.4, 500, 560, 12,  9},
        {205, "Nhà sách Nguyễn Văn Cừ",          -1.5,  -0.5, 540, 610,  8,  5},
        {206, "Bưu điện Trung tâm TP (Q.1)",      2.4,   1.5, 520, 590, 10,  6},
        {207, "Đường sách Nguyễn Văn Bình",       2.5,   1.6, 560, 630, 12,  8},
        {208, "Chợ Bến Thành (Q.1)",              1.8,   0.4, 600, 670, 10,  7},
        {209, "ĐH Bách Khoa - Lý Thường Kiệt",   -3.8,   1.8, 530, 600, 12, 10},
        {210, "ĐH Y Dược TP.HCM (Q.5)",          -2.6,  -1.2, 615, 690,  9,  5},
        {211, "ĐH Sài Gòn - Nguyễn Trãi",        -0.9,  -0.9, 660, 735, 10,  6},
        {212, "ĐH Kinh tế TP.HCM (Q.3)",          0.9,   1.9, 690, 760,  8,  4},
        {213, "Thảo Cầm Viên Sài Gòn",            3.4,   2.4, 720, 795, 10,  7},
        {214, "Landmark 81 (Bình Thạnh)",         5.2,   3.1, 750, 830, 12,  9},
        {215, "Sân bay Tân Sơn Nhất",            -1.4,   6.8, 780, 850, 15,  8},
        {216, "Crescent Mall (Phú Mỹ Hưng, Q.7)", 2.2,  -5.6, 760, 840, 12, 10},
        {217, "Lotte Mart Quận 7",                1.6,  -3.8, 830, 900, 10,  6},
        {218, "Bến xe Chợ Lớn (Q.6)",            -4.9,  -2.1, 840, 920, 10,  7}
    };
}

inline std::vector<Order> randomOrders(int n, uint64_t seed) {
    if (n < 0 || n > 200) throw std::invalid_argument("Number of orders must be 0..200");
    Rng rng(seed), rw(seed + 777);
    std::vector<Order> v;
    v.reserve(n);
    for (int i = 0; i < n; ++i) {
        int st = rng.range(480, 1080);   // 08:00 - 18:00
        int len = rng.range(40, 100);    // Độ rộng cửa sổ 40 - 100 phút
        char nameBuf[32];
        std::snprintf(nameBuf, sizeof(nameBuf), "Điểm giao %02d", i + 1);
        v.push_back({
            i + 1,
            nameBuf,
            rng.range(-80, 120) / 10.0,
            rng.range(-80, 120) / 10.0,
            st,
            st + len,
            rng.range(5, 15),
            rw.range(1, 10)
        });
    }
    return v;
}

// ----------------------------- Kiểm tra hợp lệ & Đọc/ghi CSV ------------------

inline void validateInput(const std::vector<Order>& orders, const Cfg& cfg) {
    if (orders.size() > 200) throw std::invalid_argument("Maximum 200 orders");
    if (cfg.cap < 0 || cfg.cap > 100000 || cfg.shift < 0 || cfg.shift > 10080 ||
        !std::isfinite(cfg.road) || cfg.road < 1.0 || cfg.road > 3.0) {
        throw std::invalid_argument("Invalid cap/shift/road configuration");
    }
    std::set<int> ids;
    for (const Order& o : orders) {
        if (!ids.insert(o.id).second) throw std::invalid_argument("Duplicate order ID");
        if (!std::isfinite(o.x) || !std::isfinite(o.y) || std::abs(o.x) > 10000 || std::abs(o.y) > 10000 ||
            o.start < 0 || o.start > o.end || o.end > 10080 ||
            o.service < 0 || o.service > 1440 || o.weight < 0 || o.weight > 100000) {
            throw std::invalid_argument("Invalid order values");
        }
    }
}

inline std::vector<Order> readCSV(const std::string& path) {
    std::ifstream f(std::filesystem::u8path(path), std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open CSV: " + path);

    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> row;
    std::string field;
    bool quoted = false;
    char ch;

    while (f.get(ch)) {
        if (ch == '"') {
            if (quoted && f.peek() == '"') { f.get(ch); field += '"'; }
            else quoted = !quoted;
        } else if (ch == ',' && !quoted) {
            row.push_back(field);
            field.clear();
        } else if ((ch == '\n' || ch == '\r') && !quoted) {
            if (ch == '\r' && f.peek() == '\n') f.get(ch);
            row.push_back(field);
            field.clear();
            if (row.size() > 1 || !row[0].empty()) rows.push_back(row);
            row.clear();
        } else {
            field += ch;
        }
    }
    if (quoted) throw std::invalid_argument("CSV: unclosed quote");
    row.push_back(field);
    if (row.size() > 1 || !row[0].empty()) rows.push_back(row);

    // Xóa UTF-8 BOM ở ô đầu tiên nếu có
    if (!rows.empty() && !rows[0].empty() && rows[0][0].size() >= 3 &&
        static_cast<unsigned char>(rows[0][0][0]) == 0xEF &&
        static_cast<unsigned char>(rows[0][0][1]) == 0xBB &&
        static_cast<unsigned char>(rows[0][0][2]) == 0xBF) {
        rows[0][0].erase(0, 3);
    }

    auto parseDouble = [](const std::string& text) {
        size_t pos = 0;
        double val = std::stod(text, &pos);
        if (text.find_first_not_of(" \t", pos) != std::string::npos || !std::isfinite(val))
            throw std::invalid_argument("CSV: invalid number");
        return val;
    };
    auto parseInt = [&](const std::string& text) {
        double val = parseDouble(text);
        if (val != std::trunc(val) || val < INT_MIN || val > INT_MAX)
            throw std::invalid_argument("CSV: expected integer");
        return static_cast<int>(val);
    };

    std::vector<Order> result;
    for (size_t i = 1; i < rows.size(); ++i) {
        const auto& r = rows[i];
        if (r.size() != 7 && r.size() != 8)
            throw std::invalid_argument("CSV: expected 7 or 8 columns");
        int id = parseInt(r[0]);
        std::string name = r[1].empty() ? ("Đơn " + std::to_string(id)) : r[1];
        result.push_back({
            id,
            name,
            parseDouble(r[2]),
            parseDouble(r[3]),
            parseInt(r[4]),
            parseInt(r[5]),
            parseInt(r[6]),
            r.size() == 8 ? parseInt(r[7]) : 1
        });
    }
    return result;
}

// Thoát chuỗi an toàn khi xuất JSON từ C++
inline std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out;
}
