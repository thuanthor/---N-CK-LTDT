#pragma once
// =============================================================================
// FILE: src/http_server.h
// MỤC ĐÍCH: Máy chủ HTTP nội bộ (chỉ lắng nghe 127.0.0.1) viết bằng thư viện
//           socket chuẩn của hệ điều hành (Winsock2 trên Windows, POSIX trên
//           Linux/macOS), không dùng thư viện ngoài.
//           Phục vụ các file giao diện tách riêng:
//             - web/index.html
//             - web/styles.css
//             - web/app.js
//           Và cung cấp REST API cho giao diện gọi tính toán bằng C++:
//             - GET  /api/health  : Kiểm tra kết nối máy chủ C++
//             - GET  /api/presets : Lấy bộ dữ liệu mẫu 14 đơn & TP.HCM 18 điểm
//             - POST /api/solve   : Chạy toàn bộ thuật toán đồ thị & lập lịch
// =============================================================================

#include "scheduler.h"
#include "theory.h"

#include <cctype>
#include <cstring>
#include <map>
#include <thread>

#ifdef _WIN32
  #ifndef NOMINMAX
  #define NOMINMAX
  #endif
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h>
#else
  #include <arpa/inet.h>
  #include <csignal>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
#endif

namespace web {

#ifdef _WIN32
using SocketHandle = SOCKET;
constexpr SocketHandle INVALID_SOCK = INVALID_SOCKET;

inline bool initNetwork() {
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
}

inline void closeSocket(SocketHandle s) { closesocket(s); }

inline std::string exeDir() {
    wchar_t buf[32768];
    DWORD len = GetModuleFileNameW(nullptr, buf, 32768);
    return std::filesystem::path(std::wstring(buf, len)).parent_path().u8string();
}

inline void openBrowser(const std::string& url) {
    std::string cmd = "start \"\" \"" + url + "\"";
    if (std::system(cmd.c_str())) {}
}
#else
using SocketHandle = int;
constexpr SocketHandle INVALID_SOCK = -1;

inline bool initNetwork() {
    std::signal(SIGPIPE, SIG_IGN);
    return true;
}

inline void closeSocket(SocketHandle s) { close(s); }

inline std::string exeDir() {
    char buf[4096];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0) return ".";
    buf[n] = '\0';
    std::string s(buf);
    size_t pos = s.find_last_of('/');
    return pos == std::string::npos ? "." : s.substr(0, pos);
}

inline void openBrowser(const std::string& url) {
    std::string cmd = "xdg-open '" + url + "' >/dev/null 2>&1 &";
    if (std::system(cmd.c_str())) {}
}
#endif

inline SocketHandle listenLocalhost(int& inOutPort) {
    for (int p = inOutPort; p < inOutPort + 11; ++p) {
        SocketHandle s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == INVALID_SOCK) return INVALID_SOCK;

        int reuse = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(p));
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Chỉ cho phép truy cập nội bộ 127.0.0.1

        if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0 && listen(s, 16) == 0) {
            inOutPort = p;
            return s;
        }
        closeSocket(s);
    }
    return INVALID_SOCK;
}

// ----------------------------- Định dạng JSON phụ trợ ------------------------

inline std::string fmtNum(double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.4f", v);
    return buf;
}

inline std::string jsonInts(const std::vector<int>& v) {
    std::string s = "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s += ',';
        s += std::to_string(v[i]);
    }
    return s + "]";
}

inline std::string jsonSeq(const std::vector<std::vector<int>>& v) {
    std::string s = "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) s += ',';
        s += jsonInts(v[i]);
    }
    return s + "]";
}

inline std::string orderListToJson(const std::vector<Order>& list) {
    std::ostringstream oss;
    oss << std::setprecision(8) << '[';
    for (size_t i = 0; i < list.size(); ++i) {
        const Order& o = list[i];
        if (i) oss << ',';
        oss << "{\"id\":" << o.id
            << ",\"name\":\"" << jsonEscape(o.name) << "\""
            << ",\"x\":" << o.x
            << ",\"y\":" << o.y
            << ",\"start\":" << o.start
            << ",\"end\":" << o.end
            << ",\"service\":" << o.service
            << ",\"weight\":" << o.weight << "}";
    }
    oss << ']';
    return oss.str();
}

inline std::string presetsJson() {
    return "{\"sample\":" + orderListToJson(sampleOrders()) +
           ",\"hcmc\":" + orderListToJson(hcmcOrders()) + "}";
}

// Phân loại nguyên nhân hai đơn hàng xung đột (giúp giao diện giải thích rõ khi nhấp vào đỉnh)
// 1 = Vượt tải trọng cộng gộp; 2 = Xung đột cửa sổ giờ & thời gian di chuyển; 3 = Vượt thời lượng ca
inline int classifyConflictReason(const Order& a, const Order& b, const Cfg& cfg) {
    if (cfg.exact && cfg.cap > 0 && a.weight + b.weight > cfg.cap) return 1;
    if (!cfg.exact) return 2;
    Cfg noShift = cfg;
    noShift.shift = 0;
    std::vector<Order> two = {a, b};
    double km = 0.0;
    bool okWithoutShift = simulate(two, {0, 1}, noShift, km) || simulate(two, {1, 0}, noShift, km);
    return okWithoutShift ? 3 : 2;
}

// ----------------------------- Phân tích Request & Giải bài toán -------------
// Hỗ trợ cả 2 định dạng:
//   1) Định dạng mở rộng (TSV có tên địa điểm, mạng đường lý thuyết & cạnh khóa):
//      Dòng 1: CFG <exact> <cap> <shift> <road> <roadGraphK> <theoryRoot>
//      Dòng 2 (tùy chọn): BLOCKED u1:v1 u2:v2 ...
//      Các dòng sau: id \t name \t x \t y \t start \t end \t service \t weight
//   2) Định dạng rút gọn tương thích ngược:
//      Dòng 1: <exact> <cap> <shift> <road>
//      Các dòng sau: id x y start end service weight
inline std::string solveJson(const std::string& body) {
    Cfg cfg;
    std::istringstream input(body);
    std::string line;
    if (!std::getline(input, line)) throw std::invalid_argument("Missing configuration");
    if (!line.empty() && line.back() == '\r') line.pop_back();

    std::istringstream header(line);
    std::string firstToken;
    if (!(header >> firstToken)) throw std::invalid_argument("Missing configuration");

    int exactFlag = 1;
    std::string extra;
    if (firstToken == "CFG") {
        if (!(header >> exactFlag >> cfg.cap >> cfg.shift >> cfg.road >> cfg.roadGraphK >> cfg.theoryRoot) || (header >> extra)) {
            throw std::invalid_argument("Expected: CFG exact cap shift road roadGraphK theoryRoot");
        }
    } else {
        std::istringstream legacyHeader(line);
        if (!(legacyHeader >> exactFlag >> cfg.cap >> cfg.shift >> cfg.road) || (legacyHeader >> extra)) {
            throw std::invalid_argument("Expected: exact cap shift road");
        }
    }

    if ((exactFlag != 0 && exactFlag != 1) ||
        cfg.cap < 0 || cfg.cap > 100000 ||
        cfg.shift < 0 || cfg.shift > 10080 ||
        !std::isfinite(cfg.road) || cfg.road < 1.0 || cfg.road > 3.0 ||
        cfg.roadGraphK < 0 || cfg.roadGraphK > 200 ||
        cfg.theoryRoot < 0 || cfg.theoryRoot > 200) {
        throw std::invalid_argument("Invalid configuration values");
    }
    cfg.exact = (exactFlag != 0);

    std::vector<Order> orders;
    std::set<int> ids;

    auto parseStrictDouble = [](const std::string& s) {
        size_t pos = 0;
        double v = std::stod(s, &pos);
        if (s.find_first_not_of(" \t\r", pos) != std::string::npos || !std::isfinite(v))
            throw std::invalid_argument("Invalid number");
        return v;
    };
    auto parseStrictInt = [&](const std::string& s) {
        double v = parseStrictDouble(s);
        if (v != std::trunc(v) || v < INT_MIN || v > INT_MAX)
            throw std::invalid_argument("Expected integer");
        return static_cast<int>(v);
    };

    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.find_first_not_of(" \t") == std::string::npos) continue;

        if (line.rfind("BLOCKED", 0) == 0 && (line.size() == 7 || line[7] == ' ' || line[7] == '\t')) {
            std::istringstream bs(line.substr(7));
            std::string tok;
            while (bs >> tok) {
                size_t colon = tok.find(':');
                if (colon == std::string::npos) throw std::invalid_argument("Invalid blocked edge format");
                int u = parseStrictInt(tok.substr(0, colon));
                int v = parseStrictInt(tok.substr(colon + 1));
                cfg.blockedEdges.push_back({u, v});
            }
            continue;
        }

        Order ord;
        if (line.find('\t') != std::string::npos) {
            std::vector<std::string> cols;
            std::istringstream row(line);
            std::string col;
            while (std::getline(row, col, '\t')) cols.push_back(col);
            if (!line.empty() && line.back() == '\t') cols.push_back("");
            if (cols.size() != 8) throw std::invalid_argument("Expected 8 TSV columns for order");
            ord.id = parseStrictInt(cols[0]);
            ord.name = cols[1].empty() ? ("Đơn " + std::to_string(ord.id)) : cols[1];
            ord.x = parseStrictDouble(cols[2]);
            ord.y = parseStrictDouble(cols[3]);
            ord.start = parseStrictInt(cols[4]);
            ord.end = parseStrictInt(cols[5]);
            ord.service = parseStrictInt(cols[6]);
            ord.weight = parseStrictInt(cols[7]);
        } else {
            std::istringstream row(line);
            if (!(row >> ord.id >> ord.x >> ord.y >> ord.start >> ord.end >> ord.service >> ord.weight) || (row >> extra)) {
                throw std::invalid_argument("Expected order: id x y start end service weight");
            }
            ord.name = "Đơn " + std::to_string(ord.id);
        }

        if (!ids.insert(ord.id).second) throw std::invalid_argument("Duplicate order ID");
        if (!std::isfinite(ord.x) || !std::isfinite(ord.y) || std::abs(ord.x) > 10000 || std::abs(ord.y) > 10000 ||
            ord.start < 0 || ord.end < ord.start || ord.end > 10080 ||
            ord.service < 0 || ord.service > 1440 || ord.weight < 0 || ord.weight > 100000) {
            throw std::invalid_argument("Invalid order values");
        }
        orders.push_back(std::move(ord));
        if (orders.size() > 200) throw std::invalid_argument("Maximum 200 orders");
    }

    if (orders.empty()) return "{\"empty\":true}";

    auto t0 = SteadyClock::now();
    Graph g = buildConflictGraph(orders, cfg);
    int omegaG = maxClique(g);
    int deliverableOmega = 0;
    int lb = computeDeliverableLowerBound(orders, cfg, deliverableOmega);

    struct RunRecord {
        std::string name;
        int raw = 0;
        int after = 0;
        int k = 0;
        double km = 0.0;
        Plan plan;
        std::vector<int> colors;
        std::vector<ColorStep> steps;
        double ms = 0.0;
        double colorMs = 0.0;
        bool optimal = false;
    };

    std::vector<Result> candidates = {greedyColoring(g), welshPowell(g), dsatur(g)};
    const Result& upper = *std::min_element(candidates.begin(), candidates.end(),
        [](const Result& a, const Result& b) { return a.k < b.k; });
    candidates.push_back(backtracking(g, upper, omegaG, 250000));

    std::vector<RunRecord> runs;
    int bestIdx = -1;
    for (auto& c : candidates) {
        auto runStart = SteadyClock::now();
        Plan p = repairColoring(orders, c, cfg);
        int afterRepair = p.k();
        mergeGroups(orders, p, cfg);
        relocate(orders, p, cfg);
        double totalRunMs = c.ms + std::chrono::duration<double, std::milli>(SteadyClock::now() - runStart).count();

        runs.push_back({c.name, c.k, afterRepair, p.k(), p.total(), std::move(p),
                        c.color, std::move(c.steps), totalRunMs, c.ms, c.optimal});

        if (bestIdx < 0 ||
            runs.back().k < runs[bestIdx].k ||
            (runs.back().k == runs[bestIdx].k && runs.back().km < runs[bestIdx].km)) {
            bestIdx = static_cast<int>(runs.size()) - 1;
        }
    }

    double totalMs = std::chrono::duration<double, std::milli>(SteadyClock::now() - t0).count();
    bool planValid = validatePlan(orders, runs[bestIdx].plan, cfg);
    Baseline base = solveBaseline(orders, cfg);

    std::ostringstream j;
    j << "{\"n\":" << orders.size()
      << ",\"omega\":" << deliverableOmega
      << ",\"omegaG\":" << omegaG
      << ",\"lb\":" << lb
      << ",\"valid\":" << (planValid ? "true" : "false")
      << ",\"ms\":" << fmtNum(totalMs)
      << ",\"best\":" << bestIdx
      << ",\"v1\":{\"k\":" << base.k
      << ",\"km\":" << fmtNum(base.km)
      << ",\"ok\":" << (base.capViolated ? "false" : "true") << "}"
      << ",\"edges\":[";

    bool firstEdge = true;
    for (int u = 0; u < g.n; ++u) {
        for (int v : g.adj[u]) {
            if (u < v) {
                if (!firstEdge) j << ',';
                firstEdge = false;
                j << '[' << u << ',' << v << ']';
            }
        }
    }
    j << "],\"edgeReasons\":[";
    firstEdge = true;
    for (int u = 0; u < g.n; ++u) {
        for (int v : g.adj[u]) {
            if (u < v) {
                if (!firstEdge) j << ',';
                firstEdge = false;
                j << '[' << u << ',' << v << ',' << classifyConflictReason(orders[u], orders[v], cfg) << ']';
            }
        }
    }

    j << "],\"runs\":[";
    for (size_t r = 0; r < runs.size(); ++r) {
        const auto& run = runs[r];
        std::vector<int> placed(orders.size(), 0), missing;
        for (const auto& sq : run.plan.seq) {
            for (int i : sq) placed[i]++;
        }
        for (size_t i = 0; i < orders.size(); ++i) {
            if (!placed[i]) missing.push_back(static_cast<int>(i));
        }

        if (r) j << ',';
        j << "{\"name\":\"" << jsonEscape(run.name) << "\""
          << ",\"raw\":" << run.raw
          << ",\"afterRepair\":" << run.after
          << ",\"k\":" << run.k
          << ",\"km\":" << fmtNum(run.km)
          << ",\"seq\":" << jsonSeq(run.plan.seq)
          << ",\"infeasible\":" << jsonInts(missing)
          << ",\"colors\":" << jsonInts(run.colors)
          << ",\"ms\":" << fmtNum(run.ms)
          << ",\"colorMs\":" << fmtNum(run.colorMs)
          << ",\"optimal\":" << (run.optimal ? "true" : "false")
          << ",\"valid\":" << (validatePlan(orders, run.plan, cfg) ? "true" : "false")
          << ",\"steps\":[";

        for (size_t stIdx = 0; stIdx < run.steps.size(); ++stIdx) {
            const auto& st = run.steps[stIdx];
            if (stIdx) j << ',';
            j << "{\"step\":" << st.step
              << ",\"v\":" << st.vertex
              << ",\"color\":" << st.color
              << ",\"deg\":" << st.degree
              << ",\"sat\":" << st.saturation
              << ",\"forbidden\":" << jsonInts(st.forbiddenColors)
              << ",\"reason\":\"" << jsonEscape(st.reason) << "\"}";
        }
        j << "]}";
    }

    j << "],\"vehicles\":[";
    const Plan& bestPlan = runs[bestIdx].plan;
    for (size_t c = 0; c < bestPlan.seq.size(); ++c) {
        double km = 0.0;
        std::vector<Stop> stops;
        int ret = 0;
        simulate(orders, bestPlan.seq[c], cfg, km, &stops, &ret);
        int t0Shipper = bestPlan.seq[c].empty()
            ? 0
            : std::max(0, orders[bestPlan.seq[c][0]].start - depotTime(orders[bestPlan.seq[c][0]], cfg));
        int loadKg = 0;
        for (int idx : bestPlan.seq[c]) loadKg += orders[idx].weight;

        if (c) j << ',';
        j << "{\"seq\":" << jsonInts(bestPlan.seq[c])
          << ",\"km\":" << fmtNum(km)
          << ",\"load\":" << loadKg
          << ",\"ret\":" << ret
          << ",\"t0\":" << t0Shipper
          << ",\"stops\":[";
        for (size_t i = 0; i < stops.size(); ++i) {
            if (i) j << ',';
            j << "{\"id\":" << stops[i].order
              << ",\"arrive\":" << stops[i].arrive
              << ",\"begin\":" << stops[i].begin
              << ",\"finish\":" << stops[i].finish
              << ",\"wait\":" << (stops[i].begin - stops[i].arrive) << "}";
        }
        j << "]}";
    }
    j << "],\"theory\":" << theory::buildTheoryJson(orders, g, cfg) << "}";
    return j.str();
}

// ----------------------------- Đọc file tĩnh trong thư mục web/ --------------

inline std::string readBinaryFile(const std::string& path) {
    std::ifstream f(std::filesystem::u8path(path), std::ios::binary);
    if (!f) return "";
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

inline std::string findWebDir() {
    std::string d = exeDir();
    std::vector<std::string> candidates = {
        d + "/web",
        "web",
        d + "/../web",
        "../web"
    };
    for (const std::string& dir : candidates) {
        if (!readBinaryFile(dir + "/index.html").empty()) return dir;
    }
    return "";
}

inline void sendHttpResponse(SocketHandle client, int statusCode, const char* contentType, const std::string& body) {
    const char* statusText = (statusCode == 200) ? " OK"
                           : (statusCode == 400) ? " Bad Request"
                           : (statusCode == 500) ? " Internal Server Error"
                                                 : " Not Found";
    std::string header = "HTTP/1.1 " + std::to_string(statusCode) + statusText +
                         "\r\nContent-Type: " + contentType + "; charset=utf-8"
                         "\r\nContent-Length: " + std::to_string(body.size()) +
                         "\r\nCache-Control: no-store"
                         "\r\nConnection: close\r\n\r\n";
    std::string packet = header + body;
    size_t sent = 0;
    while (sent < packet.size()) {
        int chunk = static_cast<int>(std::min<size_t>(packet.size() - sent, 65536));
        int n = send(client, packet.data() + sent, chunk, 0);
        if (n <= 0) break;
        sent += static_cast<size_t>(n);
    }
}

inline void handleClient(SocketHandle client) {
    std::string rawReq;
    char buf[8192];
    size_t headerEnd = std::string::npos;

    while ((headerEnd = rawReq.find("\r\n\r\n")) == std::string::npos) {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0 || rawReq.size() > 8000000) {
            closeSocket(client);
            return;
        }
        rawReq.append(buf, static_cast<size_t>(n));
    }

    std::string head = rawReq.substr(0, headerEnd);
    std::string lowerHead = head;
    for (char& ch : lowerHead) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

    size_t contentLength = 0;
    size_t clPos = lowerHead.find("content-length:");
    if (clPos != std::string::npos) {
        contentLength = static_cast<size_t>(std::atoll(lowerHead.c_str() + clPos + 15));
    }

    std::string body = rawReq.substr(headerEnd + 4);
    while (body.size() < contentLength && contentLength < 8000000) {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0) break;
        body.append(buf, static_cast<size_t>(n));
    }

    std::istringstream reqLine(head);
    std::string method, path;
    reqLine >> method >> path;
    std::string cleanPath = path.substr(0, path.find('?'));

    try {
        if (contentLength >= 8000000 || body.size() < contentLength) {
            throw std::invalid_argument("Invalid request size");
        }

        if (method == "POST" && cleanPath == "/api/solve") {
            sendHttpResponse(client, 200, "application/json", solveJson(body));
        } else if (method == "GET" && cleanPath == "/api/health") {
            sendHttpResponse(client, 200, "application/json", "{\"ok\":true,\"backend\":\"C++17\"}");
        } else if (method == "GET" && cleanPath == "/api/presets") {
            sendHttpResponse(client, 200, "application/json", presetsJson());
        } else if (method == "GET") {
            std::string webDir = findWebDir();
            if (cleanPath == "/styles.css" || cleanPath == "/app.css") {
                std::string css = webDir.empty() ? "" : readBinaryFile(webDir + "/styles.css");
                sendHttpResponse(client, css.empty() ? 404 : 200, "text/css", css);
            } else if (cleanPath == "/app.js") {
                std::string js = webDir.empty() ? "" : readBinaryFile(webDir + "/app.js");
                sendHttpResponse(client, js.empty() ? 404 : 200, "application/javascript", js);
            } else if (cleanPath == "/" || cleanPath == "/index.html" || cleanPath == "/giao-dien.html" ||
                       cleanPath == "/dieu-phoi" || cleanPath == "/dieu-phoi/" ||
                       cleanPath == "/do-thi" || cleanPath == "/tuyen-duong" || cleanPath == "/tuyen-duong/" ||
                       cleanPath == "/ly-thuyet" || cleanPath == "/don-hang" || cleanPath == "/don-hang/") {
                std::string html = webDir.empty() ? "" : readBinaryFile(webDir + "/index.html");
                sendHttpResponse(client, html.empty() ? 404 : 200, "text/html",
                                 html.empty() ? "Không tìm thấy thư mục giao diện web/index.html" : html);
            } else {
                sendHttpResponse(client, 404, "text/plain", "404 Not Found");
            }
        } else {
            sendHttpResponse(client, 404, "text/plain", "404 Not Found");
        }
    } catch (const std::invalid_argument& err) {
        sendHttpResponse(client, 400, "application/json",
                         std::string("{\"error\":\"") + jsonEscape(err.what()) + "\"}");
    } catch (const std::exception&) {
        sendHttpResponse(client, 500, "application/json", "{\"error\":\"Internal server error\"}");
    }
    closeSocket(client);
}

inline int runServer(bool autoOpenBrowser, int port) {
    if (!initNetwork()) {
        std::fprintf(stderr, "Không khởi tạo được kết nối mạng nội bộ (Socket).\n");
        return 1;
    }
    SocketHandle srv = listenLocalhost(port);
    if (srv == INVALID_SOCK) {
        std::fprintf(stderr, "Không mở được cổng %d (các cổng %d-%d đều đang bận).\n", port, port, port + 10);
        return 1;
    }

    std::string url = "http://127.0.0.1:" + std::to_string(port) + "/";
    std::string webDir = findWebDir();
    std::printf("===================================================================\n");
    std::printf("  DO AN LY THUYET DO THI (HCMUE) - BACKEND C++17 + WEB HTML/CSS/JS\n");
    std::printf("===================================================================\n");
    std::printf("  Thu muc Web : %s\n", webDir.empty() ? "(CHUA TIM THAY web/index.html!)" : webDir.c_str());
    std::printf("  Dia chi Web : %s\n", url.c_str());
    std::printf("  Trang thai  : %s\n", autoOpenBrowser
        ? "Trinh duyet dang tu dong mo. Giu cua so nay mo khi su dung."
        : "Hay mo dia chi tren trong trinh duyet.");
    std::printf("  Dung lai    : Nhan Ctrl+C de tat may chu.\n");
    std::printf("===================================================================\n");
    std::fflush(stdout);

    if (autoOpenBrowser) openBrowser(url);

    for (;;) {
        SocketHandle client = accept(srv, nullptr, nullptr);
        if (client == INVALID_SOCK) continue;
        std::thread(handleClient, client).detach();
    }
    return 0;
}

} // namespace web
