// =============================================================================
// ĐỒ ÁN CUỐI KỲ - HỌC PHẦN: LÝ THUYẾT ĐỒ THỊ VÀ ỨNG DỤNG (HCMUE)
// Đề tài: Ứng dụng Tô màu Đồ thị & Thuật toán Đồ thị trong Điều phối Lịch trình
//         Giao hàng Có Khung giờ cho Shipper
//
// Cấu trúc mã nguồn C++17 (thư mục src/):
//   - model.h       : Cấu trúc Order, Cfg, Graph (ma trận kề & danh sách kề), CSV
//   - coloring.h    : Greedy, Welsh-Powell, DSATUR, Backtracking + Clique cận dưới
//   - scheduler.h   : Sửa lịch, gộp shipper, tối ưu thứ tự giao, kiểm chứng độc lập
//   - theory.h      : BFS, DFS, Prim, Kruskal (DSU), Dijkstra từng bước theo slide
//   - http_server.h : Máy chủ HTTP cục bộ (127.0.0.1) phục vụ web/index.html, CSS, JS
//   - self_test.h   : Bộ kiểm thử tự động toàn diện (--self-test)
//
// Biên dịch (Windows / MinGW-w64):
//   g++ -std=c++17 -O2 -Wall -Wextra -static -o parcel.exe src/main.cpp -lws2_32
// =============================================================================

#include "self_test.h"

namespace cli {

inline void printPlanConsole(const std::vector<Order>& orders, const Plan& p, const Cfg& cfg) {
    for (int c = 0; c < p.k(); ++c) {
        std::vector<Stop> stops;
        double km = 0.0;
        int ret = 0;
        simulate(orders, p.seq[c], cfg, km, &stops, &ret);
        int w = 0;
        for (int idx : p.seq[c]) w += orders[idx].weight;
        std::printf("  Shipper %d  (%.2f km, tai %d kg, ve kho luc %s):\n",
                    c + 1, km, w, hhmm(ret).c_str());
        for (const auto& s : stops) {
            const Order& ord = orders[s.order];
            std::printf("     Don %-3d [%-28s] den %s | bat dau %s | xong %s  (cua so %s-%s)\n",
                        ord.id, ord.name.c_str(),
                        hhmm(s.arrive).c_str(), hhmm(s.begin).c_str(), hhmm(s.finish).c_str(),
                        hhmm(ord.start).c_str(), hhmm(ord.end).c_str());
        }
    }
}

inline int runBenchmark(const Cfg& cfg) {
    std::printf("n,lb,k_base,km_base,base_vi_pham,k_new,km_new,ms_new,valid\n");
    for (int n : {10, 20, 30, 50, 80, 120, 200}) {
        double lb = 0, kb = 0, mb = 0, kn = 0, mn = 0, ms = 0;
        int valid = 0, bviol = 0, runs = 20;
        for (int s = 1; s <= runs; ++s) {
            auto o = randomOrders(n, 1000 + s);
            Baseline b = solveBaseline(o, cfg);
            kb += b.k; mb += b.km; bviol += b.capViolated;
            Graph g = buildConflictGraph(o, cfg);
            Improved im = solveImproved(o, g, cfg);
            lb += im.lb; kn += im.plan.k(); mn += im.plan.total(); ms += im.ms;
            valid += validatePlan(o, im.plan, cfg);
        }
        std::printf("%d,%.2f,%.2f,%.1f,%d/%d,%.2f,%.1f,%.2f,%d/%d\n",
                    n, lb / runs, kb / runs, mb / runs, bviol, runs,
                    kn / runs, mn / runs, ms / runs, valid, runs);
        std::fflush(stdout);
    }
    return 0;
}

} // namespace cli

int applicationMain(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc == 2 && std::string(argv[1]) == "--self-test") {
        return runSelfTests();
    }
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << "Do an Ly thuyet Do thi (HCMUE) - Shipper C++17 + HTML/CSS/JS\n"
                  << "  parcel.exe [--port 8080] [--no-open]            : Mo giao dien Web\n"
                  << "  parcel.exe --self-test                          : Chay bo kiem thu tu dong\n"
                  << "  parcel.exe --sample [--cap 25 --shift 240]      : Chay mau 14 don tren dong lenh\n"
                  << "  parcel.exe --hcmc [--cap 25 --shift 240]        : Chay mau 18 diem TP.HCM\n"
                  << "  parcel.exe --random 30 2024 [--save]            : Sinh ngau nhien & giai\n"
                  << "  parcel.exe orders.csv [--save]                  : Doc file CSV & giai\n"
                  << "  parcel.exe --bench2 [--cap 25 --shift 240]      : Chay thuc nghiem hieu nang\n";
        return 0;
    }

    Cfg cfg;
    std::vector<std::string> positional;
    std::string exportPath;
    bool saveFiles = false;
    bool noOpen = false;
    int port = 8080;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--cap" && i + 1 < argc) cfg.cap = std::atoi(argv[++i]);
        else if (a == "--road" && i + 1 < argc) cfg.road = std::atof(argv[++i]);
        else if (a == "--shift" && i + 1 < argc) cfg.shift = std::atoi(argv[++i]);
        else if (a == "--conservative") cfg.exact = false;
        else if (a == "--save") saveFiles = true;
        else if (a == "--no-open") noOpen = true;
        else if (a == "--port" && i + 1 < argc) port = std::atoi(argv[++i]);
        else if (a == "--export" && i + 1 < argc) exportPath = argv[++i];
        else positional.push_back(a);
    }

    validateInput({}, cfg);
    if (port < 1 || port > 65525) {
        throw std::invalid_argument("Port must be 1..65525");
    }

    // Mặc định khi nhấp đúp mở chương trình: chạy máy chủ Web giao diện HTML/CSS/JS
    if (positional.empty()) {
        return web::runServer(!noOpen, port);
    }

    std::string mode = positional[0];
    if (mode == "--bench2") {
        return cli::runBenchmark(cfg);
    }

    std::vector<Order> orders;
    if (mode == "--random") {
        int n = positional.size() > 1 ? std::atoi(positional[1].c_str()) : 30;
        uint64_t seed = positional.size() > 2 ? static_cast<uint64_t>(std::atoll(positional[2].c_str())) : 2024;
        orders = randomOrders(n, seed);
    } else if (mode == "--sample") {
        orders = sampleOrders();
    } else if (mode == "--hcmc") {
        orders = hcmcOrders();
    } else {
        orders = readCSV(mode);
    }
    validateInput(orders, cfg);

    if (!exportPath.empty()) {
        std::ofstream f(std::filesystem::u8path(exportPath));
        f << "id,ten,x,y,bat_dau,ket_thuc,phuc_vu,khoi_luong\n";
        for (const auto& o : orders) {
            f << o.id << ",\"" << o.name << "\"," << o.x << "," << o.y << ","
              << o.start << "," << o.end << "," << o.service << "," << o.weight << "\n";
        }
    }

    Baseline base = solveBaseline(orders, cfg);
    Graph g = buildConflictGraph(orders, cfg);
    int rawLb = 0;
    auto colorings = runHeuristicColorings(g, rawLb);
    Improved im = solveImproved(orders, g, cfg);

    std::printf("=== KET QUA DIEU PHOI GIAO HANG BANG TO MAU DO THI ===\n");
    std::printf("So don hang (dinh): %d | So cap xung dot (canh): %d | Can duoi shipper: %d\n\n",
                g.n, g.edges(), im.lb);
    std::printf("--- Buoc 1: Ket qua to mau ban dau ---\n");
    for (const auto& r : colorings) {
        std::printf("  %-14s | %2d mau | %.3f ms\n", r.name.c_str(), r.k, r.ms);
    }
    std::printf("\n--- Buoc 2: Sau sua lich + gop shipper + toi uu quang duong (%s) ---\n", im.bestAlg.c_str());
    std::printf("  Phuong an goc v1   : %d shipper, %.2f km%s\n",
                base.k, base.km, base.capViolated ? " [VI PHAM TAI/CA]" : "");
    std::printf("  Phuong an cai tien : %d shipper, %.2f km | Hop le: %s | %.2f ms\n\n",
                im.plan.k(), im.plan.total(), validatePlan(orders, im.plan, cfg) ? "CO" : "KHONG", im.ms);
    cli::printPlanConsole(orders, im.plan, cfg);

    if (saveFiles) {
        std::error_code ec;
        std::filesystem::create_directories("output", ec);
        writePlanCSV(orders, g, im.plan, cfg, "output");
        std::printf("\nDa xuat ket qua ra thu muc output/ (edges.csv, assignment.csv, routes.csv).\n");
    }
    return validatePlan(orders, im.plan, cfg) ? 0 : 2;
}

#ifdef _WIN32
// Trên Windows, dùng GetCommandLineW + CommandLineToArgvW chuẩn của Win32 API
// để đọc chính xác đường dẫn tiếng Việt có dấu (ví dụ thư mục "Lý Thuyết đồ thị").
#include <shellapi.h>
int main() {
    try {
        int argc = 0;
        LPWSTR* wideArgs = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!wideArgs) throw std::runtime_error("Cannot parse Windows command line");
        std::vector<std::string> utf8Args;
        utf8Args.reserve(argc);
        for (int i = 0; i < argc; ++i) {
            int len = WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, nullptr, 0, nullptr, nullptr);
            std::string s(len > 0 ? len - 1 : 0, '\0');
            if (len > 1) WideCharToMultiByte(CP_UTF8, 0, wideArgs[i], -1, s.data(), len, nullptr, nullptr);
            utf8Args.push_back(std::move(s));
        }
        LocalFree(wideArgs);
        std::vector<char*> argv;
        for (auto& s : utf8Args) argv.push_back(s.data());
        return applicationMain(argc, argv.data());
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << '\n';
        return 1;
    }
}
#else
int main(int argc, char** argv) {
    try {
        return applicationMain(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << '\n';
        return 1;
    }
}
#endif
