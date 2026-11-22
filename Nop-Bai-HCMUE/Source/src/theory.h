#pragma once
// =============================================================================
// FILE: src/theory.h
// MỤC ĐÍCH: Cài đặt chuẩn mực các thuật toán nền tảng đối chiếu trực tiếp với
//           hai bộ slide bài giảng của TS. Nguyễn Viết Hưng (HCMUE):
//             1) BFS & DFS tạo cây/rừng khung, đếm thành phần liên thông
//                (Cây_SV2020.pdf trang 47-53, Đồ thị(1).pdf trang 46)
//             2) Cây khung nhỏ nhất Prim (Cây_SV2020.pdf trang 14-20)
//             3) Cây khung nhỏ nhất Kruskal + DSU (Cây_SV2020.pdf trang 8-13)
//             4) Đường đi ngắn nhất Dijkstra kèm BẢNG NHÃN TỪNG BƯỚC [L(v), trước(v)]
//                (Đồ thị(1).pdf trang 70-73)
//           Hỗ trợ thêm mô hình Mạng đường lân cận (k-NN) và Khóa cạnh (kẹt xe/sự cố)
//           để minh họa trực quan đường đi nhiều chặng thực tế.
// =============================================================================

#include "model.h"
#include <limits>
#include <numeric>
#include <queue>

namespace theory {

using Matrix = std::vector<std::vector<double>>;
constexpr double INF_DIST = std::numeric_limits<double>::infinity();

// 1. Kết quả duyệt đồ thị BFS / DFS (trên đồ thị xung đột G)
struct Traversal {
    int root = 0;
    std::vector<int> order;       // Thứ tự các đỉnh được thăm
    std::vector<int> parent;      // Đỉnh cha trong rừng khung (-1 nếu là gốc thành phần)
    std::vector<int> level;       // Mức (khoảng cách cạnh tính từ gốc thành phần)
    std::vector<int> component;   // Chỉ số thành phần liên thông của mỗi đỉnh
    int components = 0;           // Tổng số thành phần liên thông
};

inline Traversal traverse(const Graph& g, bool breadthFirst, int root = 0) {
    Traversal t;
    if (g.n == 0) return t;
    root = std::max(0, std::min(root, g.n - 1));
    t.root = root;
    t.parent.assign(g.n, -1);
    t.level.assign(g.n, -1);
    t.component.assign(g.n, -1);
    std::vector<char> seen(g.n, 0);

    std::function<void(int)> dfs = [&](int u) {
        seen[u] = 1;
        t.order.push_back(u);
        t.component[u] = t.components;
        for (int v = 0; v < g.n; ++v) {
            if (g.mat[u][v] && !seen[v]) {
                t.parent[v] = u;
                t.level[v] = t.level[u] + 1;
                dfs(v);
            }
        }
    };

    std::vector<int> startOrder;
    startOrder.reserve(g.n);
    startOrder.push_back(root);
    for (int v = 0; v < g.n; ++v) {
        if (v != root) startOrder.push_back(v);
    }

    for (int start : startOrder) {
        if (seen[start]) continue;
        t.level[start] = 0;
        if (breadthFirst) {
            std::queue<int> q;
            q.push(start);
            seen[start] = 1;
            while (!q.empty()) {
                int u = q.front();
                q.pop();
                t.order.push_back(u);
                t.component[u] = t.components;
                for (int v = 0; v < g.n; ++v) {
                    if (g.mat[u][v] && !seen[v]) {
                        seen[v] = 1;
                        t.parent[v] = u;
                        t.level[v] = t.level[u] + 1;
                        q.push(v);
                    }
                }
            }
        } else {
            dfs(start);
        }
        ++t.components;
    }
    return t;
}

// 2. Xây dựng ma trận trọng số khoảng cách cho Kho (đỉnh 0) + n Đơn hàng (đỉnh 1..n)
struct RoadEdgeInfo {
    int u, v;
    double w;
    bool blocked;
};

inline Matrix buildRoadMatrix(const std::vector<Order>& orders,
                              const Cfg& cfg,
                              std::vector<RoadEdgeInfo>* outRoadEdges = nullptr) {
    int n = static_cast<int>(orders.size()) + 1;
    Matrix fullDist(n, std::vector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            double x1 = i ? orders[i - 1].x : DEPOT_X;
            double y1 = i ? orders[i - 1].y : DEPOT_Y;
            double x2 = j ? orders[j - 1].x : DEPOT_X;
            double y2 = j ? orders[j - 1].y : DEPOT_Y;
            fullDist[i][j] = fullDist[j][i] = dist(x1, y1, x2, y2, cfg);
        }
    }

    // Xác định các cạnh có tồn tại trong mạng đường (đầy đủ hoặc k-láng giềng gần nhất)
    std::vector<std::vector<char>> allowed(n, std::vector<char>(n, 0));
    if (cfg.roadGraphK <= 0 || cfg.roadGraphK >= n - 1) {
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (i != j) allowed[i][j] = 1;
    } else {
        int k = cfg.roadGraphK;
        for (int u = 0; u < n; ++u) {
            std::vector<int> others;
            for (int v = 0; v < n; ++v) if (v != u) others.push_back(v);
            std::sort(others.begin(), others.end(), [&](int a, int b) {
                if (fullDist[u][a] != fullDist[u][b]) return fullDist[u][a] < fullDist[u][b];
                return a < b;
            });
            for (int idx = 0; idx < std::min(k, static_cast<int>(others.size())); ++idx) {
                int v = others[idx];
                allowed[u][v] = allowed[v][u] = 1;
            }
        }
    }

    // Đánh dấu các cạnh bị người dùng khóa (mô phỏng tắc đường / sửa đường)
    std::set<std::pair<int, int>> blockedSet;
    for (auto [u, v] : cfg.blockedEdges) {
        if (u >= 0 && v >= 0 && u < n && v < n && u != v) {
            blockedSet.insert(std::minmax(u, v));
        }
    }

    Matrix d(n, std::vector<double>(n, INF_DIST));
    for (int i = 0; i < n; ++i) d[i][i] = 0.0;

    if (outRoadEdges) outRoadEdges->clear();
    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (!allowed[u][v]) continue;
            bool isBlocked = blockedSet.count({u, v}) > 0;
            if (!isBlocked) {
                d[u][v] = d[v][u] = fullDist[u][v];
            }
            if (outRoadEdges) {
                outRoadEdges->push_back({u, v, fullDist[u][v], isBlocked});
            }
        }
    }
    return d;
}

// Hàm giữ tương thích cho kiểm thử mặc định
inline Matrix distanceMatrix(const std::vector<Order>& orders, const Cfg& cfg = Cfg{}) {
    Cfg plain = cfg;
    plain.roadGraphK = 0;
    plain.blockedEdges.clear();
    return buildRoadMatrix(orders, plain);
}

// 3. Cây khung nhỏ nhất: Prim & Kruskal
struct Edge {
    int u, v;
    double w;
};

struct Tree {
    int root = 0;
    std::vector<Edge> edges;
    double total = 0.0;
    bool connected = true;
};

// Thuật toán Prim (Cây_SV2020.pdf trang 14): mở rộng cây từ đỉnh gốc bằng cạnh nhẹ nhất nối ra ngoài
inline Tree prim(const Matrix& d, int root = 0) {
    int n = static_cast<int>(d.size());
    Tree tree;
    if (n == 0) return tree;
    root = std::max(0, std::min(root, n - 1));
    tree.root = root;

    std::vector<double> key(n, INF_DIST);
    std::vector<int> parent(n, -1);
    std::vector<char> used(n, 0);
    key[root] = 0.0;

    for (int it = 0; it < n; ++it) {
        int u = -1;
        for (int v = 0; v < n; ++v) {
            if (!used[v] && (u < 0 || key[v] < key[u])) u = v;
        }
        if (u < 0 || !std::isfinite(key[u])) break;
        used[u] = 1;
        if (parent[u] >= 0) {
            tree.edges.push_back({parent[u], u, key[u]});
            tree.total += key[u];
        }
        for (int v = 0; v < n; ++v) {
            if (!used[v] && std::isfinite(d[u][v]) && d[u][v] < key[v]) {
                key[v] = d[u][v];
                parent[v] = u;
            }
        }
    }
    tree.connected = (static_cast<int>(tree.edges.size()) == n - 1);
    return tree;
}

// Thuật toán Kruskal (Cây_SV2020.pdf trang 8): sắp cạnh tăng dần, dùng Disjoint Set Union tránh chu trình
inline Tree kruskal(const Matrix& d) {
    int n = static_cast<int>(d.size());
    Tree tree;
    if (n == 0) return tree;

    std::vector<Edge> edges;
    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (std::isfinite(d[u][v])) {
                edges.push_back({u, v, d[u][v]});
            }
        }
    }
    std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        if (a.w != b.w) return a.w < b.w;
        return std::tie(a.u, a.v) < std::tie(b.u, b.v);
    });

    std::vector<int> parent(n), rank(n, 0);
    std::iota(parent.begin(), parent.end(), 0);
    std::function<int(int)> findSet = [&](int u) {
        return parent[u] == u ? u : parent[u] = findSet(parent[u]);
    };

    for (const Edge& e : edges) {
        int a = findSet(e.u), b = findSet(e.v);
        if (a == b) continue;
        if (rank[a] < rank[b]) std::swap(a, b);
        parent[b] = a;
        if (rank[a] == rank[b]) ++rank[a];
        tree.edges.push_back(e);
        tree.total += e.w;
        if (static_cast<int>(tree.edges.size()) == n - 1) break;
    }
    tree.connected = (static_cast<int>(tree.edges.size()) == n - 1);
    return tree;
}

// 4. Thuật toán Dijkstra kèm Bảng nhãn từng bước (Đồ thị(1).pdf trang 70-73)
struct DijkstraStep {
    int lockedVertex = -1;            // Đỉnh u vừa được chốt ở vòng lặp này
    double lockedDist = 0.0;          // Nhãn ngắn nhất L(u) khi chốt
    std::vector<double> dist;         // Trạng thái nhãn L[0..n-1] sau khi cập nhật láng giềng của u
    std::vector<int> parent;          // Trạng thái đỉnh trước [0..n-1] sau khi cập nhật
    std::vector<int> relaxed;         // Danh sách các đỉnh v vừa được giảm nhãn ở bước này
};

struct ShortestPaths {
    int source = 0;
    std::vector<double> distance;
    std::vector<int> parent;
    std::vector<int> order;
    std::vector<DijkstraStep> steps;
};

inline ShortestPaths dijkstra(const Matrix& d, int source = 0) {
    int n = static_cast<int>(d.size());
    ShortestPaths s;
    if (n == 0) return s;
    source = std::max(0, std::min(source, n - 1));
    s.source = source;
    s.distance.assign(n, INF_DIST);
    s.parent.assign(n, -1);
    std::vector<char> used(n, 0);
    s.distance[source] = 0.0;

    for (int it = 0; it < n; ++it) {
        int u = -1;
        for (int v = 0; v < n; ++v) {
            if (!used[v] && (u < 0 || s.distance[v] < s.distance[u])) u = v;
        }
        if (u < 0 || !std::isfinite(s.distance[u])) break;
        used[u] = 1;
        s.order.push_back(u);

        std::vector<int> relaxed;
        for (int v = 0; v < n; ++v) {
            if (!used[v] && std::isfinite(d[u][v]) && s.distance[u] + d[u][v] < s.distance[v] - 1e-12) {
                s.distance[v] = s.distance[u] + d[u][v];
                s.parent[v] = u;
                relaxed.push_back(v);
            }
        }
        s.steps.push_back({u, s.distance[u], s.distance, s.parent, std::move(relaxed)});
    }
    return s;
}

// ----------------------------- Xuất JSON phần Lý thuyết -----------------------

inline std::string intsToJson(const std::vector<int>& v) {
    std::ostringstream s;
    s << '[';
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s << ',';
        s << v[i];
    }
    return s.str() + ']';
}

inline std::string doublesToJson(const std::vector<double>& v) {
    std::ostringstream s;
    s << std::setprecision(8) << '[';
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s << ',';
        if (std::isfinite(v[i])) s << v[i];
        else s << "null";
    }
    return s.str() + ']';
}

inline std::string traversalToJson(const Traversal& t) {
    return "{\"root\":" + std::to_string(t.root) +
           ",\"order\":" + intsToJson(t.order) +
           ",\"parent\":" + intsToJson(t.parent) +
           ",\"level\":" + intsToJson(t.level) +
           ",\"component\":" + intsToJson(t.component) +
           ",\"components\":" + std::to_string(t.components) + "}";
}

inline std::string treeToJson(const Tree& t) {
    std::ostringstream s;
    s << std::setprecision(10)
      << "{\"root\":" << t.root
      << ",\"connected\":" << (t.connected ? "true" : "false")
      << ",\"total\":" << t.total << ",\"edges\":[";
    for (size_t i = 0; i < t.edges.size(); ++i) {
        const Edge& e = t.edges[i];
        if (i) s << ',';
        s << '[' << e.u << ',' << e.v << ',' << e.w << ']';
    }
    return s.str() + "]}";
}

inline std::string buildTheoryJson(const std::vector<Order>& orders, const Graph& g, const Cfg& cfg) {
    std::vector<RoadEdgeInfo> roadEdges;
    Matrix d = buildRoadMatrix(orders, cfg, &roadEdges);

    int conflictRoot = g.n > 0 ? std::max(0, std::min(cfg.theoryRoot, g.n - 1)) : 0;
    int roadRoot = !d.empty() ? std::max(0, std::min(cfg.theoryRoot, static_cast<int>(d.size()) - 1)) : 0;

    ShortestPaths sp = dijkstra(d, roadRoot);
    Tree primTree = prim(d, roadRoot);
    Tree kruskalTree = kruskal(d);

    std::ostringstream out;
    out << std::setprecision(10);
    out << "{\"roadGraphK\":" << cfg.roadGraphK
        << ",\"bfs\":" << traversalToJson(traverse(g, true, conflictRoot))
        << ",\"dfs\":" << traversalToJson(traverse(g, false, conflictRoot))
        << ",\"degrees\":[";
    for (int v = 0; v < g.n; ++v) {
        if (v) out << ',';
        out << g.deg(v);
    }
    out << "],\"matrix\":[";
    for (int u = 0; u < g.n; ++u) {
        if (u) out << ',';
        std::vector<int> row(g.mat[u].begin(), g.mat[u].end());
        out << intsToJson(row);
    }
    out << "],\"roadEdges\":[";
    for (size_t i = 0; i < roadEdges.size(); ++i) {
        if (i) out << ',';
        out << '[' << roadEdges[i].u << ',' << roadEdges[i].v << ','
            << roadEdges[i].w << ',' << (roadEdges[i].blocked ? 1 : 0) << ']';
    }
    out << "],\"prim\":" << treeToJson(primTree)
        << ",\"kruskal\":" << treeToJson(kruskalTree)
        << ",\"dijkstra\":{\"source\":" << sp.source
        << ",\"parent\":" << intsToJson(sp.parent)
        << ",\"order\":" << intsToJson(sp.order)
        << ",\"distance\":" << doublesToJson(sp.distance)
        << ",\"steps\":[";
    for (size_t i = 0; i < sp.steps.size(); ++i) {
        const auto& st = sp.steps[i];
        if (i) out << ',';
        out << "{\"u\":" << st.lockedVertex
            << ",\"lu\":" << st.lockedDist
            << ",\"dist\":" << doublesToJson(st.dist)
            << ",\"parent\":" << intsToJson(st.parent)
            << ",\"relaxed\":" << intsToJson(st.relaxed) << "}";
    }
    out << "]}}";
    return out.str();
}

} // namespace theory
