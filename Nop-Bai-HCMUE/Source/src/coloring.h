#pragma once
// =============================================================================
// FILE: src/coloring.h
// MỤC ĐÍCH: Cài đặt 4 thuật toán Tô màu đồ thị đỉnh (Vertex Coloring) và thuật
//           toán tìm Clique lớn nhất (cận dưới số màu / số shipper):
//             1) Greedy Coloring        - O(V + E)
//             2) Welsh-Powell           - O(V log V + E)
//             3) DSATUR (Brélaz, 1979)  - O(V^2)
//             4) Backtracking (Nhánh cận chính xác)
//           Đồng thời lưu lại từng bước tô màu (ColorStep) để giao diện Web
//           minh họa trực quan kiểu Sư phạm Tin học.
// =============================================================================

#include "model.h"
#include <chrono>
#include <functional>
#include <numeric>

using SteadyClock = std::chrono::steady_clock;

// Một bước trong quá trình tô màu đồ thị (phục vụ mô phỏng từng bước trên Web)
struct ColorStep {
    int step = 0;                       // Thứ tự bước (1..n)
    int vertex = 0;                     // Chỉ số đỉnh được chọn tô (0..n-1)
    int color = 0;                      // Màu được gán (0..k-1)
    int degree = 0;                     // Bậc của đỉnh trong đồ thị xung đột
    int saturation = 0;                 // Số màu khác nhau đã xuất hiện ở các đỉnh kề trước khi tô
    std::vector<int> forbiddenColors;   // Tập các màu bị cấm do đỉnh kề đang giữ
    std::string reason;                 // Giải thích lý do chọn đỉnh và chọn màu bằng tiếng Việt
};

// Kết quả của một thuật toán tô màu
struct Result {
    std::string name;
    std::vector<int> color;             // color[v] = chỉ số màu / shipper ban đầu (0..k-1)
    int k = 0;                          // Tổng số màu sử dụng
    double ms = 0.0;                    // Thời gian thực thi (mili-giây)
    bool optimal = false;               // true nếu chứng minh được k = sắc số chi(G)
    std::vector<ColorStep> steps;       // Nhật ký từng bước tô màu
};

inline int countColors(const std::vector<int>& color) {
    int maxColor = -1;
    for (int c : color) maxColor = std::max(maxColor, c);
    return maxColor + 1;
}

// Kiểm tra phép tô màu hợp lệ: mọi đỉnh đều có màu >= 0 và hai đỉnh kề nhau khác màu
inline bool validColoring(const Graph& g, const std::vector<int>& color) {
    if (static_cast<int>(color.size()) != g.n) return false;
    for (int u = 0; u < g.n; ++u) {
        if (color[u] < 0) return false;
        for (int v : g.adj[u]) {
            if (color[u] == color[v]) return false;
        }
    }
    return true;
}

// Tìm các màu đã dùng ở láng giềng của v và trả về màu tự nhiên nhỏ nhất chưa bị cấm
inline int smallestFreeColor(const Graph& g,
                             const std::vector<int>& color,
                             int v,
                             std::vector<int>* outForbidden = nullptr) {
    std::vector<char> used(g.n + 1, 0);
    for (int u : g.adj[v]) {
        if (color[u] >= 0) used[color[u]] = 1;
    }
    if (outForbidden) {
        outForbidden->clear();
        for (int c = 0; c <= g.n; ++c) {
            if (used[c]) outForbidden->push_back(c);
        }
    }
    int c = 0;
    while (used[c]) ++c;
    return c;
}

inline std::string formatColorList(const std::vector<int>& cols) {
    if (cols.empty()) return "rỗng";
    std::ostringstream oss;
    oss << "{";
    for (size_t i = 0; i < cols.size(); ++i) {
        if (i) oss << ", ";
        oss << (cols[i] + 1);
    }
    oss << "}";
    return oss.str();
}

// 1) Thuật toán Tham lam (Greedy): duyệt theo thứ tự nhập 0..n-1, gán màu hợp lệ nhỏ nhất
inline Result greedyColoring(const Graph& g) {
    auto t0 = SteadyClock::now();
    std::vector<int> color(g.n, -1);
    std::vector<ColorStep> steps;
    steps.reserve(g.n);

    for (int v = 0; v < g.n; ++v) {
        std::vector<int> forbidden;
        int c = smallestFreeColor(g, color, v, &forbidden);
        color[v] = c;
        std::string reason = "Duyệt tuần tự theo danh sách; láng giềng đã dùng màu " +
                             formatColorList(forbidden) + " → chọn màu nhỏ nhất là Màu " + std::to_string(c + 1);
        steps.push_back({v + 1, v, c, g.deg(v), static_cast<int>(forbidden.size()), forbidden, reason});
    }

    Result r{"Greedy", color, countColors(color), 0.0, false, std::move(steps)};
    r.ms = std::chrono::duration<double, std::milli>(SteadyClock::now() - t0).count();
    return r;
}

// 2) Thuật toán Welsh-Powell: sắp xếp các đỉnh theo bậc giảm dần rồi tô tham lam
inline Result welshPowell(const Graph& g) {
    auto t0 = SteadyClock::now();
    std::vector<int> order(g.n);
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        return g.deg(a) > g.deg(b);
    });

    std::vector<int> color(g.n, -1);
    std::vector<ColorStep> steps;
    steps.reserve(g.n);

    for (int idx = 0; idx < g.n; ++idx) {
        int v = order[idx];
        std::vector<int> forbidden;
        int c = smallestFreeColor(g, color, v, &forbidden);
        color[v] = c;
        std::string reason = "Xét theo thứ tự bậc giảm dần (deg = " + std::to_string(g.deg(v)) +
                             "); màu láng giềng " + formatColorList(forbidden) +
                             " → gán Màu " + std::to_string(c + 1);
        steps.push_back({idx + 1, v, c, g.deg(v), static_cast<int>(forbidden.size()), forbidden, reason});
    }

    Result r{"Welsh-Powell", color, countColors(color), 0.0, false, std::move(steps)};
    r.ms = std::chrono::duration<double, std::milli>(SteadyClock::now() - t0).count();
    return r;
}

// 3) Thuật toán DSATUR (Degree of Saturation - Brélaz 1979):
//    Mỗi bước chọn đỉnh chưa tô có độ bão hòa sat(v) lớn nhất; nếu bằng nhau chọn bậc lớn nhất.
inline Result dsatur(const Graph& g) {
    auto t0 = SteadyClock::now();
    std::vector<int> color(g.n, -1);
    std::vector<std::set<int>> sat(g.n);
    std::vector<ColorStep> steps;
    steps.reserve(g.n);

    for (int it = 0; it < g.n; ++it) {
        int best = -1;
        for (int v = 0; v < g.n; ++v) {
            if (color[v] >= 0) continue;
            if (best < 0 ||
                sat[v].size() > sat[best].size() ||
                (sat[v].size() == sat[best].size() && g.deg(v) > g.deg(best))) {
                best = v;
            }
        }
        std::vector<int> forbidden(sat[best].begin(), sat[best].end());
        int c = smallestFreeColor(g, color, best, nullptr);
        color[best] = c;
        for (int u : g.adj[best]) {
            if (color[u] < 0) sat[u].insert(c);
        }
        std::string reason = "Chọn đỉnh chưa tô có độ bão hòa lớn nhất sat = " +
                             std::to_string(forbidden.size()) + " (deg = " + std::to_string(g.deg(best)) +
                             "); màu bị cấm " + formatColorList(forbidden) +
                             " → gán Màu " + std::to_string(c + 1);
        steps.push_back({it + 1, best, c, g.deg(best), static_cast<int>(forbidden.size()), forbidden, reason});
    }

    Result r{"DSATUR", color, countColors(color), 0.0, false, std::move(steps)};
    r.ms = std::chrono::duration<double, std::milli>(SteadyClock::now() - t0).count();
    return r;
}

// Tìm kích thước Clique lớn nhất omega(G) bằng nhánh cận (cận dưới cho sắc số chi(G) >= omega(G))
inline int maxClique(const Graph& g, long long nodeLimit = 2000000) {
    int best = 0;
    long long nodes = 0;
    std::function<void(std::vector<int>&, std::vector<int>)> expand =
        [&](std::vector<int>& R, std::vector<int> P) {
            if (++nodes > nodeLimit) return;
            best = std::max(best, static_cast<int>(R.size()));
            if (R.size() + P.size() <= static_cast<size_t>(best)) return;
            while (!P.empty()) {
                if (R.size() + P.size() <= static_cast<size_t>(best)) return;
                int v = P.back();
                P.pop_back();
                std::vector<int> nextP;
                for (int u : P) {
                    if (g.mat[v][u]) nextP.push_back(u);
                }
                R.push_back(v);
                expand(R, std::move(nextP));
                R.pop_back();
            }
        };

    std::vector<int> R;
    std::vector<int> P(g.n);
    std::iota(P.begin(), P.end(), 0);
    expand(R, std::move(P));
    return best;
}

// 4) Thuật toán Quay lui - Nhánh cận (Backtracking) tìm số màu tối thiểu chính xác
struct BacktrackSolver {
    const Graph& g;
    std::vector<int> order;
    std::vector<int> cur;
    std::vector<int> best;
    int bestK;
    long long nodes = 0;
    long long limit;
    bool aborted = false;
    int lowerBound;

    BacktrackSolver(const Graph& g_, int ub, const std::vector<int>& ubColor, long long lim, int lb)
        : g(g_), cur(g_.n, -1), best(ubColor), bestK(ub), limit(lim), lowerBound(lb) {
        order.resize(g.n);
        std::iota(order.begin(), order.end(), 0);
        std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
            return g.deg(a) > g.deg(b);
        });
    }

    void search(int idx, int usedColors) {
        if (aborted || bestK == lowerBound) return;
        if (++nodes > limit) { aborted = true; return; }
        if (usedColors >= bestK) return; // Cắt tỉa nhánh không tốt hơn nghiệm hiện có
        if (idx == g.n) {
            bestK = usedColors;
            best = cur;
            return;
        }
        int v = order[idx];
        std::vector<char> forbidden(usedColors + 1, 0);
        for (int u : g.adj[v]) {
            if (cur[u] >= 0) forbidden[cur[u]] = 1;
        }
        for (int c = 0; c <= usedColors && c < bestK; ++c) {
            if (c < usedColors && forbidden[c]) continue;
            cur[v] = c;
            search(idx + 1, std::max(usedColors, c + 1));
            cur[v] = -1;
            if (aborted) return;
        }
    }
};

inline Result backtracking(const Graph& g, const Result& upperBound, int lowerBound, long long limit = 5000000) {
    auto t0 = SteadyClock::now();
    BacktrackSolver bt(g, upperBound.k, upperBound.color, limit, lowerBound);
    bt.search(0, 0);

    std::vector<ColorStep> steps;
    steps.reserve(g.n);
    std::vector<int> partial(g.n, -1);
    for (int idx = 0; idx < g.n; ++idx) {
        int v = bt.order[idx];
        int c = bt.best[v];
        std::set<int> adjCols;
        for (int u : g.adj[v]) {
            if (partial[u] >= 0) adjCols.insert(partial[u]);
        }
        partial[v] = c;
        std::vector<int> forbidden(adjCols.begin(), adjCols.end());
        std::string reason = (!bt.aborted || bt.bestK == lowerBound)
            ? ("Nghiệm tối ưu từ cây quay lui nhánh cận (đã chứng minh χ(G) = " + std::to_string(bt.bestK) + ") → Màu " + std::to_string(c + 1))
            : ("Nghiệm tốt nhất tìm được trong giới hạn " + std::to_string(limit) + " nút → Màu " + std::to_string(c + 1));
        steps.push_back({idx + 1, v, c, g.deg(v), static_cast<int>(forbidden.size()), forbidden, reason});
    }

    Result r{"Backtracking", bt.best, bt.bestK, 0.0, !bt.aborted || bt.bestK == lowerBound, std::move(steps)};
    r.ms = std::chrono::duration<double, std::milli>(SteadyClock::now() - t0).count();
    return r;
}

inline std::vector<Result> runAllColorings(const Graph& g, int& outLb, long long btLimit = 5000000) {
    Result a = greedyColoring(g);
    Result b = welshPowell(g);
    Result c = dsatur(g);
    const Result* best = &a;
    if (b.k < best->k) best = &b;
    if (c.k < best->k) best = &c;
    outLb = maxClique(g);
    Result d = backtracking(g, *best, outLb, btLimit);
    return {a, b, c, d};
}

inline std::vector<Result> runHeuristicColorings(const Graph& g, int& outLb) {
    outLb = maxClique(g);
    return {greedyColoring(g), welshPowell(g), dsatur(g)};
}
