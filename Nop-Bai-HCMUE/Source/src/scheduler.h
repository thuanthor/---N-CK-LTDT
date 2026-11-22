#pragma once
// =============================================================================
// FILE: src/scheduler.h
// MỤC ĐÍCH: Chuyển kết quả Tô màu đồ thị thành Lịch trình giao hàng hợp lệ và
//           tối ưu hóa quãng đường:
//             - Bước 1 (repairColoring): Sửa lịch theo cửa sổ giờ, tải trọng, ca làm
//             - Bước 2 (mergeGroups)   : Gộp các nhóm nhỏ để giảm số lượng shipper
//             - Bước 3 (relocate)      : Chuyển đơn giữa các shipper để giảm tổng km
//             - Kiểm chứng độc lập (validatePlan): Đối chiếu lại toàn bộ ràng buộc
// =============================================================================

#include "coloring.h"

// Kế hoạch phân phối hoàn chỉnh cho các shipper
struct Plan {
    std::vector<std::vector<int>> seq;   // seq[c] = danh sách chỉ số đơn theo thứ tự giao của shipper c
    std::vector<double> km;              // km[c]  = quãng đường đi + về kho của shipper c
    std::vector<int> infeasible;         // Các chỉ số đơn không thể giao dù đi một mình

    double total() const {
        double s = 0.0;
        for (double d : km) s += d;
        return s;
    }

    int k() const { return static_cast<int>(seq.size()); }
};

// Xây dựng lộ trình cơ sở từ kết quả tô màu (chỉ sắp xếp đơn theo giờ mở cửa tăng dần)
inline std::vector<Route> buildRoutes(const std::vector<Order>& orders, const Result& r, const Cfg& cfg) {
    std::vector<Route> routes(r.k);
    for (int c = 0; c < r.k; ++c) {
        std::vector<int> ids;
        for (size_t i = 0; i < orders.size(); ++i) {
            if (r.color[i] == c) ids.push_back(static_cast<int>(i));
        }
        std::sort(ids.begin(), ids.end(), [&](int a, int b) {
            return orders[a].start < orders[b].start;
        });

        Route& rt = routes[c];
        double cx = DEPOT_X, cy = DEPOT_Y;
        int t = ids.empty() ? 0 : std::max(0, orders[ids[0]].start - depotTime(orders[ids[0]], cfg));

        for (int id : ids) {
            double d = dist(cx, cy, orders[id].x, orders[id].y, cfg);
            int arrive = t + static_cast<int>(std::ceil(d / SPEED_KM_PER_MIN));
            int begin = std::max(arrive, orders[id].start);
            if (begin > orders[id].end) rt.feasible = false;
            int finish = begin + orders[id].service;
            rt.stops.push_back({id, arrive, begin, finish});
            rt.km += d;
            t = finish;
            cx = orders[id].x;
            cy = orders[id].y;
        }
        double back = dist(cx, cy, DEPOT_X, DEPOT_Y, cfg);
        rt.km += back;
        rt.returnTime = t + static_cast<int>(std::ceil(back / SPEED_KM_PER_MIN));
    }
    return routes;
}

// Tìm thứ tự giao hàng có quãng đường ngắn nhất (đúng khung giờ & ca làm) cho một tập đơn:
// - Nếu số đơn <= 7: Vét cạn nhánh cận (Branch & Bound) tìm thứ tự tối ưu chính xác.
// - Nếu số đơn > 7 : Khởi tạo theo khung giờ rồi tìm kiếm cục bộ (chuyển vị trí 1 đơn).
inline bool bestSequence(const std::vector<Order>& orders,
                         const std::vector<int>& ids,
                         const Cfg& cfg,
                         std::vector<int>& outBest,
                         double& outBestKm) {
    int m = static_cast<int>(ids.size());
    outBestKm = 1e18;
    outBest.clear();
    if (m == 0) {
        outBestKm = 0.0;
        return true;
    }

    if (m <= 7) {
        std::vector<int> cur;
        std::vector<char> used(m, 0);
        std::function<void(int, double, double, double, int, int)> dfs =
            [&](int cnt, double cx, double cy, double km, int t, int t0) {
                if (km >= outBestKm) return; // Nhánh cận: quãng đường tích lũy đã lớn hơn nghiệm tốt nhất
                if (cnt == m) {
                    double back = dist(cx, cy, DEPOT_X, DEPOT_Y, cfg);
                    double totalKm = km + back;
                    int returnTime = t + static_cast<int>(std::ceil(back / SPEED_KM_PER_MIN));
                    if (cfg.shift > 0 && returnTime - t0 > cfg.shift) return;
                    if (totalKm < outBestKm) {
                        outBestKm = totalKm;
                        outBest = cur;
                    }
                    return;
                }
                for (int i = 0; i < m; ++i) {
                    if (used[i]) continue;
                    const Order& ord = orders[ids[i]];
                    double d = dist(cx, cy, ord.x, ord.y, cfg);
                    int departOrPrev = (cnt == 0) ? std::max(0, ord.start - depotTime(ord, cfg)) : t;
                    int arrive = departOrPrev + static_cast<int>(std::ceil(d / SPEED_KM_PER_MIN));
                    int begin = std::max(arrive, ord.start);
                    if (begin > ord.end) continue;
                    int shiftStart = (cnt == 0) ? departOrPrev : t0;
                    if (cfg.shift > 0 && begin + ord.service - shiftStart > cfg.shift) continue;

                    used[i] = 1;
                    cur.push_back(ids[i]);
                    dfs(cnt + 1, ord.x, ord.y, km + d, begin + ord.service, shiftStart);
                    cur.pop_back();
                    used[i] = 0;
                }
            };
        dfs(0, DEPOT_X, DEPOT_Y, 0.0, 0, 0);
        return !outBest.empty();
    }

    // Heuristic cho nhóm lớn (> 7 đơn)
    std::vector<int> seq = ids;
    double km = 0.0;
    std::sort(seq.begin(), seq.end(), [&](int a, int b) {
        return orders[a].start < orders[b].start;
    });
    if (!simulate(orders, seq, cfg, km)) {
        std::sort(seq.begin(), seq.end(), [&](int a, int b) {
            return orders[a].end < orders[b].end;
        });
        if (!simulate(orders, seq, cfg, km)) return false;
    }

    bool improved = true;
    int guard = 0;
    while (improved && guard++ < 20) {
        improved = false;
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j) {
                if (i == j) continue;
                std::vector<int> cand = seq;
                int moved = cand[i];
                cand.erase(cand.begin() + i);
                cand.insert(cand.begin() + j, moved);
                double candKm = 0.0;
                if (simulate(orders, cand, cfg, candKm) && candKm < km - 1e-9) {
                    seq = std::move(cand);
                    km = candKm;
                    improved = true;
                }
            }
        }
    }
    outBest = std::move(seq);
    outBestKm = km;
    return true;
}

// Kiểm tra một tập đơn có thể do 1 shipper đảm nhận (đủ tải trọng + thời gian + ca làm) hay không
inline bool groupOK(const std::vector<Order>& orders,
                    const std::vector<int>& ids,
                    const Cfg& cfg,
                    std::vector<int>& outSeq,
                    double& outKm) {
    if (cfg.cap > 0) {
        int totalWeight = 0;
        for (int i : ids) totalWeight += orders[i].weight;
        if (totalWeight > cfg.cap) return false;
    }
    return bestSequence(orders, ids, cfg, outSeq, outKm);
}

// Chèn đơn id vào shipper hiện có sao cho độ tăng quãng đường là nhỏ nhất
inline int insertBest(const std::vector<Order>& orders, Plan& p, int id, const Cfg& cfg, int skipGroup = -1) {
    int bestGroup = -1;
    double bestDelta = 1e18;
    std::vector<int> bestSeq;
    double bestKm = 0.0;

    for (int g = 0; g < p.k(); ++g) {
        if (g == skipGroup) continue;
        std::vector<int> candIds = p.seq[g];
        candIds.push_back(id);
        std::vector<int> candSeq;
        double candKm = 0.0;
        if (groupOK(orders, candIds, cfg, candSeq, candKm) && (candKm - p.km[g] < bestDelta)) {
            bestDelta = candKm - p.km[g];
            bestGroup = g;
            bestSeq = std::move(candSeq);
            bestKm = candKm;
        }
    }
    if (bestGroup >= 0) {
        p.seq[bestGroup] = std::move(bestSeq);
        p.km[bestGroup] = bestKm;
    }
    return bestGroup;
}

inline bool addNewGroup(const std::vector<Order>& orders, Plan& p, int id, const Cfg& cfg) {
    std::vector<int> seq;
    double km = 0.0;
    if (!groupOK(orders, {id}, cfg, seq, km)) return false;
    p.seq.push_back(std::move(seq));
    p.km.push_back(km);
    return true;
}

// Bước 1: Sửa kết quả tô màu thành kế hoạch giao hàng thực sự hợp lệ
inline Plan repairColoring(const std::vector<Order>& orders, const Result& r, const Cfg& cfg) {
    Plan p;
    std::vector<int> pending;

    for (int c = 0; c < r.k; ++c) {
        std::vector<int> ids;
        for (size_t i = 0; i < orders.size(); ++i) {
            if (r.color[i] == c) ids.push_back(static_cast<int>(i));
        }
        std::sort(ids.begin(), ids.end(), [&](int a, int b) {
            return orders[a].start < orders[b].start;
        });

        std::vector<int> acceptedIds, acceptedSeq, candSeq;
        double acceptedKm = 0.0, candKm = 0.0;
        for (int id : ids) {
            std::vector<int> trial = acceptedIds;
            trial.push_back(id);
            if (groupOK(orders, trial, cfg, candSeq, candKm)) {
                acceptedIds = std::move(trial);
                acceptedSeq = std::move(candSeq);
                acceptedKm = candKm;
            } else {
                pending.push_back(id);
            }
        }
        if (!acceptedIds.empty()) {
            p.seq.push_back(std::move(acceptedSeq));
            p.km.push_back(acceptedKm);
        }
    }

    std::sort(pending.begin(), pending.end(), [&](int a, int b) {
        return orders[a].start < orders[b].start;
    });
    for (int id : pending) {
        if (insertBest(orders, p, id, cfg) < 0 && !addNewGroup(orders, p, id, cfg)) {
            p.infeasible.push_back(id); // Đơn vượt tải hoặc vượt ca ngay cả khi đi riêng 1 shipper
        }
    }
    return p;
}

// Bước 2: Gộp shipper — thử giải tán nhóm ít đơn nhất và san sang các shipper còn lại
inline void mergeGroups(const std::vector<Order>& orders, Plan& p, const Cfg& cfg) {
    bool changed = true;
    while (changed && p.k() > 1) {
        changed = false;
        std::vector<int> groupOrder(p.k());
        std::iota(groupOrder.begin(), groupOrder.end(), 0);
        std::sort(groupOrder.begin(), groupOrder.end(), [&](int a, int b) {
            return p.seq[a].size() < p.seq[b].size();
        });

        for (int g : groupOrder) {
            Plan trial = p;
            std::vector<int> moving = p.seq[g];
            std::sort(moving.begin(), moving.end(), [&](int a, int b) {
                return orders[a].start < orders[b].start;
            });
            bool allInserted = true;
            for (int id : moving) {
                if (insertBest(orders, trial, id, cfg, g) < 0) {
                    allInserted = false;
                    break;
                }
            }
            if (allInserted) {
                trial.seq.erase(trial.seq.begin() + g);
                trial.km.erase(trial.km.begin() + g);
                p = std::move(trial);
                changed = true;
                break;
            }
        }
    }
}

// Bước 3: Chuyển đơn giữa các shipper (Local Search Relocate) để giảm tổng quãng đường
inline void relocate(const std::vector<Order>& orders, Plan& p, const Cfg& cfg) {
    for (int pass = 0; pass < 5; ++pass) {
        bool improved = false;
        for (int a = 0; a < p.k(); ++a) {
            for (size_t pos = 0; pos < p.seq[a].size(); ++pos) {
                int id = p.seq[a][pos];
                std::vector<int> remA = p.seq[a];
                remA.erase(remA.begin() + pos);
                std::vector<int> seqA;
                double kmA = 0.0;
                if (!remA.empty() && !groupOK(orders, remA, cfg, seqA, kmA)) continue;

                for (int b = 0; b < p.k(); ++b) {
                    if (b == a) continue;
                    std::vector<int> addB = p.seq[b];
                    addB.push_back(id);
                    std::vector<int> seqB;
                    double kmB = 0.0;
                    if (!groupOK(orders, addB, cfg, seqB, kmB)) continue;

                    if (kmA + kmB < p.km[a] + p.km[b] - 1e-6) {
                        p.seq[b] = std::move(seqB);
                        p.km[b] = kmB;
                        improved = true;
                        if (remA.empty()) {
                            p.seq.erase(p.seq.begin() + a);
                            p.km.erase(p.km.begin() + a);
                            --a;
                        } else {
                            p.seq[a] = std::move(seqA);
                            p.km[a] = kmA;
                        }
                        goto nextShipperA;
                    }
                }
            }
            nextShipperA:;
        }
        if (!improved) break;
    }
}

// Kiểm chứng ĐỘC LẬP toàn bộ lịch trình (không gọi lại simulate hay groupOK):
// - Mọi đơn trong danh sách được giao đúng 1 lần (không thiếu, không trùng)
// - Đúng cửa sổ thời gian [start, end]
// - Không vượt tải trọng cfg.cap và thời lượng ca cfg.shift
// - Quãng đường khớp chính xác với p.km
inline bool validatePlan(const std::vector<Order>& orders, const Plan& p, const Cfg& cfg) {
    if (p.seq.size() != p.km.size()) return false;
    std::vector<int> deliveredCount(orders.size(), 0);

    for (size_t c = 0; c < p.seq.size(); ++c) {
        const auto& seq = p.seq[c];
        if (seq.empty()) return false;
        for (int i : seq) {
            if (i < 0 || i >= static_cast<int>(orders.size())) return false;
        }

        const Order& first = orders[seq[0]];
        int departure = std::max(0, first.start - static_cast<int>(std::ceil(std::hypot(first.x, first.y) * cfg.road / SPEED_KM_PER_MIN)));
        int curTime = departure;
        int totalWeight = 0;
        double x = DEPOT_X, y = DEPOT_Y, totalKm = 0.0;

        for (int i : seq) {
            const Order& ord = orders[i];
            double d = std::hypot(x - ord.x, y - ord.y) * cfg.road;
            int begin = std::max(curTime + static_cast<int>(std::ceil(d / SPEED_KM_PER_MIN)), ord.start);
            if (begin > ord.end) return false;
            curTime = begin + ord.service;
            x = ord.x;
            y = ord.y;
            totalKm += d;
            totalWeight += ord.weight;
            ++deliveredCount[i];
        }

        double back = std::hypot(x - DEPOT_X, y - DEPOT_Y) * cfg.road;
        totalKm += back;
        int returnTime = curTime + static_cast<int>(std::ceil(back / SPEED_KM_PER_MIN));

        if (cfg.cap > 0 && totalWeight > cfg.cap) return false;
        if (cfg.shift > 0 && returnTime - departure > cfg.shift) return false;
        if (std::abs(totalKm - p.km[c]) > 1e-7 * std::max(1.0, totalKm)) return false;
    }

    return std::all_of(deliveredCount.begin(), deliveredCount.end(), [](int cnt) { return cnt == 1; });
}

// Tính cận dưới chặt cho số shipper trên tập các đơn khả thi (loại trừ các đơn lẻ bất khả thi để không làm sai lệch clique)
inline int computeDeliverableLowerBound(const std::vector<Order>& orders, const Cfg& cfg, int& outExactOmega) {
    std::vector<Order> feasibleOrders;
    feasibleOrders.reserve(orders.size());
    for (const Order& ord : orders) {
        if (isSingleOrderFeasible(ord, cfg)) feasibleOrders.push_back(ord);
    }
    if (feasibleOrders.empty()) {
        outExactOmega = 0;
        return 0;
    }
    Cfg exactCfg = cfg;
    exactCfg.exact = true;
    Graph gExact = buildConflictGraph(feasibleOrders, exactCfg);
    outExactOmega = maxClique(gExact);
    int lb = outExactOmega;
    if (cfg.cap > 0) {
        int sumWeight = 0;
        for (const Order& ord : feasibleOrders) sumWeight += ord.weight;
        lb = std::max(lb, (sumWeight + cfg.cap - 1) / cfg.cap);
    }
    return std::max(lb, 1);
}

struct Improved {
    Plan plan;
    int kBefore = 0;
    std::string bestAlg;
    int lb = 0;
    double ms = 0.0;
};

inline Improved solveImproved(const std::vector<Order>& orders, const Graph& g, const Cfg& cfg) {
    auto t0 = SteadyClock::now();
    int rawLb = 0;
    auto colorings = runHeuristicColorings(g, rawLb);

    Improved im;
    int exactOmega = 0;
    im.lb = computeDeliverableLowerBound(orders, cfg, exactOmega);

    double bestKm = 1e18;
    bool first = true;
    for (const auto& r : colorings) {
        Plan p = repairColoring(orders, r, cfg);
        mergeGroups(orders, p, cfg);
        relocate(orders, p, cfg);
        if (first || p.k() < im.plan.k() || (p.k() == im.plan.k() && p.total() < bestKm)) {
            im.plan = std::move(p);
            bestKm = im.plan.total();
            im.bestAlg = r.name;
            im.kBefore = r.k;
            first = false;
        }
    }
    im.ms = std::chrono::duration<double, std::milli>(SteadyClock::now() - t0).count();
    return im;
}

// Phương án gốc v1 (chỉ dùng điều kiện bảo toàn, không xét tải/ca lúc tô màu, không tối ưu thứ tự)
struct Baseline {
    int k = 0;
    double km = 0.0;
    bool capViolated = false;
};

inline Baseline solveBaseline(const std::vector<Order>& orders, const Cfg& actualCfg) {
    Cfg v1Cfg = actualCfg;
    v1Cfg.exact = false;
    v1Cfg.cap = 0;
    v1Cfg.shift = 0;

    Graph g = buildConflictGraph(orders, v1Cfg);
    int lb = 0;
    auto rs = runAllColorings(g, lb);
    const Result* best = &rs[0];
    for (const auto& r : rs) {
        if (r.k < best->k) best = &r;
    }

    auto routes = buildRoutes(orders, *best, actualCfg);
    double totalKm = 0.0;
    Plan pl;
    for (const auto& rt : routes) {
        totalKm += rt.km;
        std::vector<int> seq;
        for (const auto& st : rt.stops) seq.push_back(st.order);
        pl.seq.push_back(std::move(seq));
        pl.km.push_back(rt.km);
    }
    bool ok = validatePlan(orders, pl, actualCfg);
    return {best->k, totalKm, !ok};
}

inline void writePlanCSV(const std::vector<Order>& orders, const Graph& g, const Plan& p, const Cfg& cfg, const std::string& dir) {
    {
        std::ofstream f(std::filesystem::u8path(dir + "/edges.csv"));
        f << "u,v\n";
        for (int u = 0; u < g.n; ++u)
            for (int v : g.adj[u])
                if (u < v) f << orders[u].id << "," << orders[v].id << "\n";
    }
    std::ofstream a(std::filesystem::u8path(dir + "/assignment.csv"));
    std::ofstream r(std::filesystem::u8path(dir + "/routes.csv"));
    a << "id,name,shipper,x,y,start,end,service,weight\n";
    r << "shipper,order_id,name,arrive,begin,finish\n";
    for (int c = 0; c < p.k(); ++c) {
        std::vector<Stop> stops;
        double km = 0.0;
        simulate(orders, p.seq[c], cfg, km, &stops, nullptr);
        for (const auto& s : stops) {
            const Order& ord = orders[s.order];
            r << (c + 1) << "," << ord.id << ",\"" << ord.name << "\"," << s.arrive << "," << s.begin << "," << s.finish << "\n";
            a << ord.id << ",\"" << ord.name << "\"," << (c + 1) << "," << ord.x << "," << ord.y << ","
              << ord.start << "," << ord.end << "," << ord.service << "," << ord.weight << "\n";
        }
    }
}
