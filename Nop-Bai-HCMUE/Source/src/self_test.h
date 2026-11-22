#pragma once
// =============================================================================
// FILE: src/self_test.h
// MỤC ĐÍCH: Bộ kiểm thử tích hợp tự động (--self-test) kiểm chứng:
//             1) Vét cạn toàn bộ 64 đồ thị vô hướng 4 đỉnh: Clique, Sắc số,
//                Greedy/Welsh-Powell/DSATUR, BFS/DFS rừng khung, Định lý bắt tay.
//             2) Các bước tô màu từng bước (ColorStep) của Greedy, WP, DSATUR.
//             3) Prim, Kruskal, Dijkstra (kèm bảng nhãn từng bước), mạng đường
//                lân cận k-NN và khóa cạnh kẹt xe.
//             4) Lịch giao trên bộ mẫu chuẩn, kiểm chứng độc lập validatePlan,
//                cận dưới chính xác khi có đơn bất khả thi, và giao thức API TSV.
// =============================================================================

#include "http_server.h"
#include <iostream>

inline int runSelfTests() {
    int checks = 0;
    auto require = [&](bool ok, const std::string& label) {
        ++checks;
        if (!ok) throw std::runtime_error("FAIL: " + label);
    };

    try {
        // 1. Vét cạn mọi đơn đồ thị vô hướng 4 đỉnh: 2^6 = 64 đồ thị
        std::vector<std::pair<int, int>> pairs;
        for (int u = 0; u < 4; ++u)
            for (int v = u + 1; v < 4; ++v)
                pairs.push_back({u, v});

        for (int mask = 0; mask < 64; ++mask) {
            Graph g;
            g.init(4);
            for (int bit = 0; bit < 6; ++bit) {
                if (mask & (1 << bit)) g.addEdge(pairs[bit].first, pairs[bit].second);
            }

            int oracleClique = 0;
            for (int subset = 1; subset < 16; ++subset) {
                bool isClique = true;
                for (int u = 0; u < 4; ++u) {
                    for (int v = u + 1; v < 4; ++v) {
                        if ((subset & (1 << u)) && (subset & (1 << v)) && !g.mat[u][v]) {
                            isClique = false;
                        }
                    }
                }
                if (isClique) oracleClique = std::max(oracleClique, __builtin_popcount(static_cast<unsigned>(subset)));
            }

            int oracleColors = 4;
            for (int code = 0; code < 256; ++code) {
                std::vector<int> c(4);
                int val = code;
                for (int& x : c) { x = val % 4; val /= 4; }
                if (validColoring(g, c)) oracleColors = std::min(oracleColors, countColors(c));
            }

            require(maxClique(g) == oracleClique, "clique exhaustive");
            Result gRes = greedyColoring(g);
            Result wpRes = welshPowell(g);
            Result dsRes = dsatur(g);
            Result btRes = backtracking(g, dsRes, oracleClique);

            require(btRes.k == oracleColors && btRes.optimal && validColoring(g, btRes.color), "chromatic exhaustive");
            for (const Result& r : {gRes, wpRes, dsRes}) {
                require(validColoring(g, r.color) && static_cast<int>(r.steps.size()) == 4, "heuristic coloring + step trace");
            }

            auto bfs = theory::traverse(g, true), dfs = theory::traverse(g, false);
            require(bfs.order.size() == 4 && dfs.order.size() == 4 && bfs.components == dfs.components, "BFS/DFS forest");

            int degreeSum = 0;
            for (int v = 0; v < 4; ++v) degreeSum += g.deg(v);
            require(degreeSum == 2 * g.edges(), "handshake theorem");
        }

        // 2. Kiểm tra Prim, Kruskal, Dijkstra (kèm bảng nhãn từng bước) trên đồ thị gián tiếp
        theory::Matrix d = {
            {0, 1, 5, 10},
            {1, 0, 2,  5},
            {5, 2, 0,  1},
            {10, 5, 1, 0}
        };
        auto primTree = theory::prim(d);
        auto kruskalTree = theory::kruskal(d);
        auto sp = theory::dijkstra(d, 0);

        require(primTree.edges.size() == 3 && std::abs(primTree.total - 4.0) < 1e-9, "Prim known tree");
        require(kruskalTree.edges.size() == 3 && std::abs(kruskalTree.total - 4.0) < 1e-9, "Kruskal known tree");
        require(std::abs(sp.distance[3] - 4.0) < 1e-9 && sp.parent[3] == 2, "Dijkstra indirect path");
        require(sp.steps.size() == 4 && sp.steps[0].lockedVertex == 0 && std::abs(sp.steps[2].dist[3] - 4.0) < 1e-9, "Dijkstra step-by-step table");

        // 3. Kiểm tra thứ tự duyệt BFS/DFS với đỉnh gốc tùy chọn và đỉnh cô lập
        Graph branch;
        branch.init(5);
        branch.addEdge(0, 1);
        branch.addEdge(0, 2);
        branch.addEdge(1, 3);
        require(theory::traverse(branch, true, 0).order == std::vector<int>({0, 1, 2, 3, 4}), "BFS order and isolated vertex");
        require(theory::traverse(branch, false, 0).order == std::vector<int>({0, 1, 3, 2, 4}), "DFS order and isolated vertex");
        require(theory::traverse(branch, true, 1).order[0] == 1, "BFS custom root vertex");

        // 4. Kiểm tra bộ dữ liệu 14 đơn chuẩn trên 3 cấu hình tham chiếu
        auto orders = sampleOrders();
        for (int mode = 0; mode < 3; ++mode) {
            Cfg cfg;
            if (mode > 0) { cfg.cap = 25; cfg.shift = 240; }
            if (mode == 2) { cfg.road = 1.3; }
            auto g = buildConflictGraph(orders, cfg);
            auto im = solveImproved(orders, g, cfg);
            int expectedK[] = {1, 4, 5};
            double expectedKm[] = {130.08, 157.88, 181.53};
            require(validatePlan(orders, im.plan, cfg), "sample independent validation");
            require(im.plan.k() == expectedK[mode] && std::abs(im.plan.total() - expectedKm[mode]) < 0.005, "sample reference");
            auto matrix = theory::distanceMatrix(orders, cfg);
            require(std::abs(theory::prim(matrix).total - theory::kruskal(matrix).total) < 1e-8, "MST agreement on sample");
        }

        // 5. Kiểm tra mạng đường lân cận k-NN & khóa cạnh kẹt xe
        {
            Cfg roadCfg;
            roadCfg.roadGraphK = 3;
            roadCfg.blockedEdges = {{0, 6}}; // Khóa đường trực tiếp từ Kho đến Đơn 6
            std::vector<theory::RoadEdgeInfo> edgesInfo;
            auto roadMat = theory::buildRoadMatrix(orders, roadCfg, &edgesInfo);
            require(!std::isfinite(roadMat[0][6]), "blocked road edge applied");
            auto spRoad = theory::dijkstra(roadMat, 0);
            require(std::isfinite(spRoad.distance[6]) && spRoad.parent[6] != 0, "Dijkstra reroutes via intermediate vertex");
        }

        // 6. Kiểm tra sửa lịch theo tổng tải trọng và các trường hợp biên của validatePlan
        {
            Cfg capCfg;
            capCfg.cap = 10;
            std::vector<Order> three = {
                {1, "A", 0, 0, 0, 100, 1, 4},
                {2, "B", 0, 0, 0, 100, 1, 4},
                {3, "C", 0, 0, 0, 100, 1, 4}
            };
            auto g = buildConflictGraph(three, capCfg);
            auto p = solveImproved(three, g, capCfg).plan;
            require(g.edges() == 0 && p.k() == 2 && validatePlan(three, p, capCfg), "pairwise compatible but total capacity repair");

            Plan missing;
            require(!validatePlan(three, missing, capCfg), "reject missing orders");
            Plan duplicate;
            duplicate.seq = {{0, 0}};
            duplicate.km = {0};
            require(!validatePlan(three, duplicate, capCfg), "reject duplicates");
        }

        // 7. Kiểm tra bộ mẫu TP.HCM 18 điểm & các bộ sinh ngẫu nhiên
        {
            Cfg hcmcCfg;
            hcmcCfg.cap = 25;
            hcmcCfg.shift = 240;
            hcmcCfg.road = 1.25;
            auto hcmc = hcmcOrders();
            auto g = buildConflictGraph(hcmc, hcmcCfg);
            auto p = solveImproved(hcmc, g, hcmcCfg).plan;
            require(validatePlan(hcmc, p, hcmcCfg), "HCMC 18-landmark schedule valid");
        }
        for (int seed : {1, 17, 2024}) {
            Cfg rndCfg;
            rndCfg.cap = 25;
            rndCfg.shift = 240;
            rndCfg.road = 1.3;
            auto o = randomOrders(20, seed);
            auto g = buildConflictGraph(o, rndCfg);
            auto p = solveImproved(o, g, rndCfg).plan;
            require(validatePlan(o, p, rndCfg), "seeded random schedules");
        }

        // 8. Kiểm tra API: đơn bất khả thi, cận dưới không bị thổi phồng, TSV có tên tiếng Việt
        {
            std::string ans1 = web::solveJson("1 25 240 1\n1 0 0 500 600 5 30\n");
            require(ans1.find("\"valid\":false") != std::string::npos &&
                    ans1.find("\"infeasible\":[0]") != std::string::npos, "infeasible order reported");

            std::string ans2 = web::solveJson(
                "CFG 1 25 240 1.2 3 0\n"
                "BLOCKED 0:1\n"
                "101\tĐH Sư Phạm CS1\t-1.2\t0.8\t500\t600\t10\t30\n"
                "102\tChợ Bến Thành\t1.5\t0.4\t520\t620\t10\t5\n"
            );
            require(ans2.find("\"lb\":1") != std::string::npos &&
                    ans2.find("\"infeasible\":[0]") != std::string::npos, "tight lower bound excludes infeasible single order");

            bool rejected = false;
            try {
                web::solveJson("1 25 240 1\n1 0 0 600 500 5 1\n");
            } catch (const std::invalid_argument&) {
                rejected = true;
            }
            require(rejected, "invalid time window rejected");
            require(web::solveJson("1 0 0 1\n") == "{\"empty\":true}", "empty API input");
        }

        std::cout << "PASS: " << checks
                  << " checks (64 exhaustive graphs, step traces, Dijkstra table, road network, schedules, API).\n";
        return 0;
    } catch (const std::exception& err) {
        std::cerr << err.what() << '\n';
        return 1;
    }
}
