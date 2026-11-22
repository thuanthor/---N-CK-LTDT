'use strict';
// =============================================================================
// FILE: web/app.js
// MỤC ĐÍCH: Điều khiển giao diện người dùng (Frontend) và trực quan hóa đồ thị.
//           Toàn bộ kết quả tính toán (Đồ thị xung đột, 4 thuật toán Tô màu,
//           Sửa/Tối ưu lịch, Kiểm chứng độc lập, BFS/DFS, Prim/Kruskal, Bảng
//           nhãn từng bước Dijkstra) đều được lấy trực tiếp từ máy chủ C++17
//           thông qua API POST /api/solve.
// =============================================================================

(() => {
  const $ = (id) => document.getElementById(id);
  const STORAGE_KEY = 'shipper_graph_workspace_v2';

  const PALETTE = [
    '#4f46e5', '#d97706', '#0d9488', '#db2777', '#2563eb',
    '#7c3aed', '#dc2626', '#059669', '#9333ea', '#ea580c'
  ];
  const colorOf = (i) => PALETTE[((i % PALETTE.length) + PALETTE.length) % PALETTE.length];

  const esc = (v) => String(v ?? '').replace(/[&<>"']/g, (ch) => ({
    '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;'
  }[ch]));

  const fmtTime = (m) => {
    const h = Math.floor(m / 60);
    const min = Math.round(m % 60);
    return `${String(h).padStart(2, '0')}:${String(min).padStart(2, '0')}`;
  };

  const parseTimeMins = (str) => {
    const m = /^(\d{1,3}):(\d{2})$/.exec(String(str).trim());
    return m && Number(m[2]) < 60 ? Number(m[1]) * 60 + Number(m[2]) : NaN;
  };

  const fmtNum = (n, digits = 2) =>
    Number(n).toLocaleString('vi-VN', { minimumFractionDigits: digits, maximumFractionDigits: digits });

  // Bộ 14 đơn mẫu dự phòng khi khởi tạo trước khi nhận /api/presets
  const DEFAULT_SAMPLE = [
    { id: 101, name: 'ĐH Sư Phạm CS1 (Q.5)',        x: -4.8, y:  6.9, start: 574, end: 641, service: 12, weight: 10 },
    { id: 102, name: 'Cảng Khánh Hội (Q.4)',         x:  6.4, y: -6.0, start: 687, end: 732, service:  7, weight:  6 },
    { id: 103, name: 'CV Phần mềm Quang Trung',     x: -8.0, y: 11.2, start: 646, end: 708, service: 15, weight:  1 },
    { id: 104, name: 'Bến xe Miền Tây',             x: -6.0, y:  5.6, start: 522, end: 620, service: 12, weight: 10 },
    { id: 105, name: 'Sân bay Tân Sơn Nhất',        x: -1.6, y: 11.7, start: 752, end: 802, service:  7, weight:  7 },
    { id: 106, name: 'Chợ Bến Thành (Q.1)',          x:  4.3, y: -0.4, start: 547, end: 646, service: 13, weight:  5 },
    { id: 107, name: 'Hồ Con Rùa (Q.3)',             x:  1.4, y:  5.8, start: 913, end: 972, service: 11, weight:  1 },
    { id: 108, name: 'Ngã tư Phú Nhuận',             x:  0.9, y:  8.8, start: 817, end: 897, service:  5, weight:  8 },
    { id: 109, name: 'ĐH Tôn Đức Thắng (Q.7)',      x: -1.8, y: -7.9, start: 523, end: 589, service:  8, weight: 10 },
    { id: 110, name: 'Chợ Xóm Chiếu (Q.4)',         x: -0.2, y: -4.0, start: 920, end: 990, service:  7, weight:  8 },
    { id: 111, name: 'Chợ Bà Chiểu (Bình Thạnh)',    x:  2.4, y:  7.7, start: 863, end: 910, service: 15, weight:  3 },
    { id: 112, name: 'Khu Công nghệ cao (Thủ Đức)', x: 11.5, y: 10.6, start: 887, end: 967, service:  6, weight:  8 },
    { id: 113, name: 'KDC Trung Sơn (Bình Chánh)',  x: -5.1, y: -7.1, start: 1060, end: 1114, service: 5, weight: 9 },
    { id: 114, name: 'ĐH Sư Phạm CS2 (Q.3)',        x: -3.5, y: -1.8, start: 665, end: 758, service: 10, weight:  1 }
  ];

  // Trạng thái toàn cục của giao diện
  let presets = { sample: DEFAULT_SAMPLE, hcmc: [] };
  let orders = DEFAULT_SAMPLE.map((o) => ({ ...o }));
  let blockedEdges = []; // Mảng các cặp [u, v] bị khóa trên mạng đường lý thuyết
  let result = null;

  let activeTab = 'map';
  let selectedShipper = null;
  let inspectedVertex = null;
  let mapAddMode = false;
  let zoom = 100;
  let mapTransform = null;
  let editIndex = -1;

  // Trạng thái chạy từng bước & mô phỏng xe
  let colorStep = Infinity;
  let colorPlayTimer = null;
  let theoryStep = Infinity;
  let theoryPlayTimer = null;
  let simMinutes = null;
  let simPlayTimer = null;

  // Điều phối mạng & thông báo
  let solveDebounce = null;
  let abortCtrl = null;
  let reqRevision = 0;
  let toastTimer = null;

  // Kéo-thả đỉnh trên bản đồ
  let dragState = null;

  // ----------------------------- Tiện ích chung ------------------------------

  function notify(msg) {
    clearTimeout(toastTimer);
    const box = $('toastBox');
    box.textContent = msg;
    box.hidden = false;
    toastTimer = setTimeout(() => { box.hidden = true; }, 3400);
  }

  function setConnectionStatus(ok) {
    const badge = $('connectionBadge');
    badge.className = `conn-badge ${ok ? 'ok' : 'bad'}`;
    badge.innerHTML = `<span class="conn-dot"></span>${ok ? 'C++17 hoạt động' : 'Mất kết nối C++'}`;
  }

  function readConfig() {
    return {
      exact: Number($('conflictModeSelect').value),
      cap: Number($('capacityInput').value),
      shift: Number($('shiftInput').value),
      road: Number($('roadInput').value),
      roadGraphK: Number($('roadGraphKSelect').value),
      theoryRoot: Math.max(0, Number($('theoryRootSelect').value || 0))
    };
  }

  function writeConfigToInputs(cfg) {
    if (!cfg) return;
    $('conflictModeSelect').value = cfg.exact ? '1' : '0';
    $('capacityInput').value = cfg.cap ?? 25;
    $('shiftInput').value = cfg.shift ?? 240;
    $('roadInput').value = cfg.road ?? 1;
    if (cfg.roadGraphK !== undefined) {
      $('roadGraphKSelect').value = String(cfg.roadGraphK);
    }
  }

  function validateOrdersAndConfig(list, cfg = readConfig()) {
    if (!Array.isArray(list) || list.length > 200) {
      throw new Error('Số lượng đơn hàng tối đa là 200 đơn.');
    }
    if (!Number.isInteger(cfg.cap) || cfg.cap < 0 || cfg.cap > 100000 ||
        !Number.isInteger(cfg.shift) || cfg.shift < 0 || cfg.shift > 10080 ||
        !Number.isFinite(cfg.road) || cfg.road < 1 || cfg.road > 3) {
      throw new Error('Tải trọng/Ca phải là số nguyên không âm; Hệ số đường trong khoảng [1.0, 3.0].');
    }
    const seenIds = new Set();
    for (const o of list) {
      if (!o || !Number.isInteger(o.id) || seenIds.has(o.id)) {
        throw new Error('Mã đơn phải là số nguyên và không được trùng nhau.');
      }
      seenIds.add(o.id);
      if (!Number.isFinite(o.x) || !Number.isFinite(o.y) || Math.abs(o.x) > 10000 || Math.abs(o.y) > 10000) {
        throw new Error(`Đơn #${o.id}: Tọa độ X, Y phải nằm trong [-10000, 10000] km.`);
      }
      if (![o.start, o.end, o.service, o.weight].every(Number.isInteger) ||
          o.start < 0 || o.end < o.start || o.end > 10080 ||
          o.service < 0 || o.service > 1440 || o.weight < 0 || o.weight > 100000) {
        throw new Error(`Đơn #${o.id}: Kiểm tra lại khung giờ (mở <= đóng), thời gian phục vụ và khối lượng.`);
      }
    }
  }

  function saveWorkspace() {
    try {
      validateOrdersAndConfig(orders);
      const payload = JSON.stringify({
        version: 2,
        orders,
        config: readConfig(),
        blockedEdges,
        autoSolve: $('autoSolveCheck').checked
      });
      localStorage.setItem(STORAGE_KEY, payload);
    } catch {
      // Không ghi đè nếu dữ liệu đang nhập dở chưa hợp lệ
    }
  }

  function loadWorkspace() {
    try {
      const raw = localStorage.getItem(STORAGE_KEY);
      if (!raw) return false;
      const data = JSON.parse(raw);
      if (data.version !== 2 || !Array.isArray(data.orders)) return false;
      validateOrdersAndConfig(data.orders, data.config);
      orders = data.orders.map((o) => ({
        ...o,
        name: String(o.name || `Đơn ${o.id}`)
      }));
      blockedEdges = Array.isArray(data.blockedEdges) ? data.blockedEdges : [];
      writeConfigToInputs(data.config);
      $('autoSolveCheck').checked = Boolean(data.autoSolve);
      return true;
    } catch {
      return false;
    }
  }

  function stopAllTimers() {
    if (colorPlayTimer) { clearInterval(colorPlayTimer); colorPlayTimer = null; $('colorStepPlay').textContent = '▶ Tự chạy'; }
    if (theoryPlayTimer) { clearInterval(theoryPlayTimer); theoryPlayTimer = null; $('theoryStepPlay').textContent = '▶ Tự chạy'; }
    if (simPlayTimer) { clearInterval(simPlayTimer); simPlayTimer = null; $('simPlayBtn').textContent = '▶ Mô phỏng xe chạy'; }
  }

  function populateTheoryVertexSelectors() {
    const kind = $('theoryAlgSelect').value;
    const isTraversal = (kind === 'bfs' || kind === 'dfs');
    const prevRoot = Number($('theoryRootSelect').value || 0);
    const prevTarget = Number($('theoryTargetSelect').value || 1);

    if (isTraversal) {
      $('theoryRootSelect').innerHTML = orders.map((o, i) =>
        `<option value="${i}">#${o.id} · ${esc(o.name)}</option>`
      ).join('');
      $('theoryRootSelect').value = String(Math.min(prevRoot, Math.max(0, orders.length - 1)));
    } else {
      const opts = [`<option value="0">Kho trung tâm (0, 0)</option>`].concat(
        orders.map((o, i) => `<option value="${i + 1}">#${o.id} · ${esc(o.name)}</option>`)
      );
      $('theoryRootSelect').innerHTML = opts.join('');
      $('theoryRootSelect').value = String(Math.min(prevRoot, orders.length));

      $('theoryTargetSelect').innerHTML = opts.join('');
      $('theoryTargetSelect').value = String(Math.min(Math.max(1, prevTarget), orders.length));
    }
    $('theoryTargetWrap').hidden = (kind !== 'dijkstra');
    $('roadNetworkWrap').hidden = isTraversal;
    $('theoryClickHint').hidden = isTraversal;
    updateBlockedButton();
  }

  function updateBlockedButton() {
    const btn = $('clearBlockedBtn');
    const isTraversal = ['bfs', 'dfs'].includes($('theoryAlgSelect').value);
    btn.hidden = isTraversal || blockedEdges.length === 0;
    btn.textContent = `Mở lại các đường bị khóa (${blockedEdges.length})`;
  }

  function invalidateData(resetBlocked = false) {
    reqRevision++;
    abortCtrl?.abort();
    clearTimeout(solveDebounce);
    stopAllTimers();
    if (resetBlocked) blockedEdges = [];
    result = null;
    selectedShipper = null;
    inspectedVertex = null;
    simMinutes = null;
    colorStep = Infinity;
    theoryStep = Infinity;
    $('vertexInspectBox').hidden = true;

    populateTheoryVertexSelectors();
    renderAll();
    saveWorkspace();

    if ($('autoSolveCheck').checked && orders.length > 0) {
      solveDebounce = setTimeout(solveWithCpp, 180);
    }
  }

  // ----------------------------- Gọi máy chủ C++17 ---------------------------

  async function loadPresetsFromCpp() {
    try {
      const res = await fetch('/api/presets');
      if (!res.ok) return;
      const data = await res.json();
      if (Array.isArray(data.sample) && data.sample.length) {
        presets.sample = data.sample.map((o) => ({ ...o, id: o.id + 100 }));
      }
      if (Array.isArray(data.hcmc) && data.hcmc.length) {
        presets.hcmc = data.hcmc;
      }
    } catch {
      // Giữ bộ mẫu dự phòng nếu chưa kết nối
    }
  }

  async function solveWithCpp() {
    clearTimeout(solveDebounce);
    try {
      validateOrdersAndConfig(orders);
    } catch (err) {
      $('alertsBox').innerHTML = `<div class="banner bad">${esc(err.message)}</div>`;
      return;
    }

    saveWorkspace();
    if (orders.length === 0) {
      result = null;
      renderAll();
      return;
    }

    abortCtrl?.abort();
    abortCtrl = new AbortController();
    const myRev = ++reqRevision;

    $('solveBtn').disabled = true;
    $('solveBtn').textContent = 'Đang tính C++...';
    $('alertsBox').innerHTML = '';

    const cfg = readConfig();
    const headerLine = `CFG ${cfg.exact} ${cfg.cap} ${cfg.shift} ${cfg.road} ${cfg.roadGraphK} ${cfg.theoryRoot}`;
    const blockedLine = blockedEdges.length
      ? `BLOCKED ${blockedEdges.map(([u, v]) => `${u}:${v}`).join(' ')}\n`
      : '';
    const orderLines = orders.map((o) => {
      const cleanName = String(o.name || `Đơn ${o.id}`).replace(/[\t\r\n]+/g, ' ').trim();
      return [o.id, cleanName, o.x, o.y, o.start, o.end, o.service, o.weight].join('\t');
    }).join('\n');

    try {
      const res = await fetch('/api/solve', {
        method: 'POST',
        headers: { 'Content-Type': 'text/plain; charset=utf-8' },
        body: `${headerLine}\n${blockedLine}${orderLines}`,
        signal: abortCtrl.signal
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || `Lỗi HTTP ${res.status}`);
      if (myRev !== reqRevision) return;

      result = data.empty ? null : data;
      setConnectionStatus(true);

      // Cập nhật danh sách thuật toán tô màu
      if (result) {
        const prevAlg = $('colorAlgSelect').value;
        $('colorAlgSelect').innerHTML = result.runs.map((r, i) =>
          `<option value="${i}">${esc(r.name)} (${r.raw} màu → ${r.k} shipper)</option>`
        ).join('');
        $('colorAlgSelect').value = (prevAlg !== '' && result.runs[Number(prevAlg)]) ? prevAlg : String(result.best);

        // Cập nhật khoảng thanh trượt mô phỏng thời gian thực
        if (result.vehicles.length) {
          const minT = Math.floor(Math.min(...result.vehicles.map((v) => v.t0)) / 10) * 10;
          const maxT = Math.ceil(Math.max(...result.vehicles.map((v) => v.ret)) / 10) * 10;
          $('simTimeSlider').min = String(minT);
          $('simTimeSlider').max = String(Math.max(minT + 10, maxT));
          if (simMinutes === null) $('simTimeSlider').value = String(Math.max(minT + 10, maxT));
        }
      }
      renderAll();
    } catch (err) {
      if (err.name === 'AbortError' || myRev !== reqRevision) return;
      result = null;
      setConnectionStatus(false);
      renderAll();
      $('alertsBox').innerHTML = `<div class="banner bad">Không kết nối được máy chủ C++ (${esc(err.message)}). Hãy mở chương trình bằng <b>MO-GIAO-DIEN.cmd</b>.</div>`;
    } finally {
      if (myRev === reqRevision) {
        $('solveBtn').disabled = false;
        $('solveBtn').textContent = 'Tối ưu bằng C++';
      }
    }
  }

  // ----------------------------- Render Tổng thể -----------------------------

  function bestRun() {
    return result ? result.runs[result.best] : null;
  }

  function vehicles() {
    return result?.vehicles || [];
  }

  function visibleVehicles() {
    return vehicles()
      .map((v, index) => ({ ...v, index }))
      .filter((v) => selectedShipper === null || v.index === selectedShipper);
  }

  function renderAll() {
    const cfg = readConfig();
    const totalKg = orders.reduce((s, o) => s + o.weight, 0);
    const best = bestRun();

    // 1. KPI Cards
    $('kpiOrders').textContent = String(orders.length);
    $('kpiWeightSub').textContent = `Tổng khối lượng: ${totalKg} kg`;
    $('kpiShippers').textContent = best ? `${best.k} shipper` : (orders.length ? '—' : '0');
    $('kpiBoundSub').textContent = result
      ? `Cận dưới lý thuyết: ${result.lb} (Clique ω = ${result.omega})`
      : 'Cận dưới lý thuyết: —';
    $('kpiDistance').textContent = best ? `${fmtNum(best.km)} km` : (orders.length ? '— km' : '0 km');
    $('kpiRoadSub').textContent = cfg.road === 1
      ? 'Khoảng cách đường thẳng ×1.00'
      : `Đã nhân hệ số đường thực tế ×${fmtNum(cfg.road)}`;

    if (!orders.length) {
      $('kpiStatus').textContent = 'Chưa có dữ liệu';
      $('kpiStatus').className = 'kpi-status';
      $('kpiTimeSub').textContent = 'Hãy thêm đơn hoặc chọn bộ mẫu';
    } else if (!result) {
      $('kpiStatus').textContent = 'Chưa chạy C++';
      $('kpiStatus').className = 'kpi-status';
      $('kpiTimeSub').textContent = 'Bấm "Tối ưu bằng C++"';
    } else if (result.valid) {
      $('kpiStatus').textContent = '✓ Hợp lệ 100%';
      $('kpiStatus').className = 'kpi-status text-good';
      $('kpiTimeSub').textContent = `C++17 xử lý: ${fmtNum(result.ms)} ms (${best.name})`;
    } else {
      $('kpiStatus').textContent = '✕ Có đơn không giao được';
      $('kpiStatus').className = 'kpi-status text-bad';
      $('kpiTimeSub').textContent = `C++17 xử lý: ${fmtNum(result.ms)} ms`;
    }

    // 2. Cảnh báo ràng buộc
    let banners = '';
    if (result && best) {
      if (best.infeasible.length > 0) {
        const names = best.infeasible.map((idx) => `#${orders[idx].id} (${orders[idx].name})`).join(', ');
        banners += `<div class="banner bad"><b>Phát hiện ${best.infeasible.length} đơn vượt quá khả năng phục vụ:</b> ${esc(names)}. Các đơn này quá tải trọng tối đa hoặc không kịp ca làm việc ngay cả khi đi riêng 1 shipper, nên không được tính vào lịch hợp lệ.</div>`;
      } else if (!result.valid) {
        banners += `<div class="banner bad">Lịch trình chưa vượt qua hàm kiểm chứng độc lập <code>validatePlan()</code>.</div>`;
      }
      if (!cfg.exact) {
        banners += `<div class="banner warn">Bạn đang bật <b>Đồ thị bảo toàn (v1)</b> để so sánh lý thuyết. Lịch cuối cùng vẫn được C++ sửa chữa và kiểm chứng theo khung giờ, tải trọng và ca thực tế.</div>`;
      }
    }
    $('alertsBox').innerHTML = banners;

    // 3. Bảng đơn nhanh ở thanh bên trái
    $('quickOrderCount').textContent = `${orders.length} đơn`;
    $('quickOrdersBody').innerHTML = orders.map((o, i) => `
      <tr>
        <td>
          <button type="button" class="quick-order-btn" data-quick-edit="${i}" title="Nhấp để sửa đơn #${o.id}">
            <strong class="mono">#${o.id}</strong>
            <small>${esc(o.name)}</small>
          </button>
        </td>
        <td class="mono">${fmtTime(o.start)}–${fmtTime(o.end)}</td>
        <td class="mono">${o.weight}</td>
        <td>
          <button type="button" class="btn btn-xs" data-quick-del="${i}" title="Xóa đơn #${o.id}">✕</button>
        </td>
      </tr>
    `).join('') || `<tr><td colspan="4" class="hint-text" style="text-align:center;padding:18px">Danh sách trống</td></tr>`;

    // 4. Thanh lọc Shipper
    $('shipperFilterBar').hidden = !['map', 'schedule'].includes(activeTab);
    $('solverMetaText').textContent = result
      ? `Thuật toán gốc tốt nhất: ${best.name} · Tô màu ${best.raw} → Lịch cuối ${best.k} shipper`
      : 'Chưa có lịch trình';
    $('shipperFilterButtons').innerHTML = vehicles().length
      ? `<button type="button" class="pill-btn" data-shipper-filter="all" aria-pressed="${selectedShipper === null}">Tất cả (${vehicles().length})</button>` +
        vehicles().map((v, i) => `
          <button type="button" class="pill-btn" data-shipper-filter="${i}" aria-pressed="${selectedShipper === null || selectedShipper === i}">
            <span class="color-dot" style="background:${colorOf(i)}"></span>S${i + 1} (${v.seq.length} đơn)
          </button>
        `).join('')
      : '<span class="hint-text">Chưa có shipper</span>';

    // 5. Vẽ nội dung tab đang mở
    if (activeTab === 'map') renderMap();
    else if (activeTab === 'graph') renderConflictGraph();
    else if (activeTab === 'schedule') renderScheduleAndGantt();
    else if (activeTab === 'theory') renderTheory();
    else if (activeTab === 'orders') renderOrdersManager();
  }

  // ----------------------------- Hệ tọa độ SVG -------------------------------

  function computeGeoPositions(points) {
    const xs = points.map((p) => p.x);
    const ys = points.map((p) => p.y);
    const minX = Math.min(0, ...xs), maxX = Math.max(0, ...xs);
    const minY = Math.min(0, ...ys), maxY = Math.max(0, ...ys);
    const cx = (minX + maxX) / 2;
    const cy = (minY + maxY) / 2;
    const spanX = Math.max(6, maxX - minX + 3.5);
    const spanY = Math.max(6, maxY - minY + 3.5);
    const scale = Math.min(760 / spanX, 410 / spanY);

    const toX = (x) => 460 + (x - cx) * scale;
    const toY = (y) => 270 - (y - cy) * scale;
    const fromX = (px) => (px - 460) / scale + cx;
    const fromY = (py) => (270 - py) / scale + cy;
    mapTransform = { toX, toY, fromX, fromY };
    return points.map((p) => ({ x: toX(p.x), y: toY(p.y) }));
  }

  function computeCirclePositions(count) {
    if (count === 0) return [];
    const rx = 310, ry = 205;
    return Array.from({ length: count }, (_, i) => {
      const angle = (2 * Math.PI * i) / count - Math.PI / 2;
      return { x: 460 + rx * Math.cos(angle), y: 270 + ry * Math.sin(angle) };
    });
  }

  function directedSegmentSvg(a, b, color, width = 2.6, markerId = '', dashed = false, opacity = 1) {
    const dx = b.x - a.x, dy = b.y - a.y;
    const len = Math.hypot(dx, dy);
    if (len < 1e-3) return '';
    // Rút ngắn điểm cuối để đầu mũi tên chạm đúng viền ngoài hình tròn bán kính 16px
    const padEnd = markerId && len > 28 ? 18 : 0;
    const x2 = b.x - (dx / len) * padEnd;
    const y2 = b.y - (dy / len) * padEnd;
    return `<line x1="${a.x.toFixed(1)}" y1="${a.y.toFixed(1)}" x2="${x2.toFixed(1)}" y2="${y2.toFixed(1)}"
      stroke="${color}" stroke-width="${width}" stroke-opacity="${opacity}"
      ${dashed ? 'stroke-dasharray="7 5"' : ''}
      ${markerId ? `marker-end="url(#${markerId})"` : ''}/>`;
  }

  // ----------------------------- TAB 1: Bản đồ & Mô phỏng --------------------

  function computeShipperStateAtMinute(v, minute, pts) {
    // pts[0] = Kho, pts[i+1] = orders[i]
    if (minute <= v.t0) {
      return { x: pts[0].x, y: pts[0].y, label: `Chờ xuất kho (${fmtTime(v.t0)})` };
    }
    if (minute >= v.ret) {
      return { x: pts[0].x, y: pts[0].y, label: `Đã hoàn thành ca (${fmtTime(v.ret)})` };
    }
    let prevPt = pts[0];
    let prevTime = v.t0;
    for (const s of v.stops) {
      const targetPt = pts[s.id + 1];
      const ord = orders[s.id];
      if (minute <= s.arrive) {
        const ratio = s.arrive > prevTime ? (minute - prevTime) / (s.arrive - prevTime) : 1;
        return {
          x: prevPt.x + (targetPt.x - prevPt.x) * ratio,
          y: prevPt.y + (targetPt.y - prevPt.y) * ratio,
          label: `Đang đi giao #${ord.id}`
        };
      }
      if (minute <= s.begin) {
        return { x: targetPt.x, y: targetPt.y, label: `Chờ giờ mở cửa #${ord.id}` };
      }
      if (minute <= s.finish) {
        return { x: targetPt.x, y: targetPt.y, label: `Đang giao #${ord.id}` };
      }
      prevPt = targetPt;
      prevTime = s.finish;
    }
    const ratio = v.ret > prevTime ? (minute - prevTime) / (v.ret - prevTime) : 1;
    return {
      x: prevPt.x + (pts[0].x - prevPt.x) * ratio,
      y: prevPt.y + (pts[0].y - prevPt.y) * ratio,
      label: 'Đang quay về Kho'
    };
  }

  function renderMap() {
    $('mapEmptyState').hidden = orders.length > 0;
    $('mapCanvasWrap').classList.toggle('map-adding-cursor', mapAddMode);
    $('mapAddModeBtn').setAttribute('aria-pressed', String(mapAddMode));

    const allPoints = [{ x: 0, y: 0 }, ...orders];
    const pts = computeGeoPositions(allPoints);

    // Định nghĩa đầu mũi tên cho từng màu shipper
    const defs = `<defs>${PALETTE.map((c, idx) => `
      <marker id="arrow-${idx}" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
        <path d="M 0 1.5 L 9 5 L 0 8.5 z" fill="${c}"></path>
      </marker>
    `).join('')}</defs>`;

    let svgBody = '';

    // Trục tọa độ mờ đi qua Kho (0, 0)
    svgBody += `<line x1="0" y1="${pts[0].y.toFixed(1)}" x2="920" y2="${pts[0].y.toFixed(1)}" stroke="var(--line)" stroke-dasharray="4 4"/>`;
    svgBody += `<line x1="${pts[0].x.toFixed(1)}" y1="0" x2="${pts[0].x.toFixed(1)}" y2="540" stroke="var(--line)" stroke-dasharray="4 4"/>`;

    // Vẽ lộ trình từng shipper
    const assignedMap = new Map();
    vehicles().forEach((v, c) => v.seq.forEach((ordIdx, stopIdx) => assignedMap.set(ordIdx, { c, stopIdx })));

    for (const v of visibleVehicles()) {
      const col = colorOf(v.index);
      const markerId = `arrow-${v.index % PALETTE.length}`;
      const routeNodes = [0, ...v.seq.map((i) => i + 1)];
      const fadeOpacity = simMinutes !== null ? 0.42 : 0.92;

      for (let j = 1; j < routeNodes.length; ++j) {
        svgBody += directedSegmentSvg(pts[routeNodes[j - 1]], pts[routeNodes[j]], col, 2.7, markerId, false, fadeOpacity);
      }
      if (routeNodes.length > 1) {
        svgBody += directedSegmentSvg(pts[routeNodes[routeNodes.length - 1]], pts[0], col, 2.0, markerId, true, fadeOpacity * 0.85);
      }
    }

    // Vẽ các đỉnh đơn hàng
    orders.forEach((o, i) => {
      const info = assignedMap.get(i);
      if (selectedShipper !== null && info?.c !== selectedShipper) return;
      const p = pts[i + 1];
      const fill = info ? colorOf(info.c) : '#7c869b';
      const shortName = o.name.length > 18 ? o.name.slice(0, 17) + '…' : o.name;
      const tooltip = `#${o.id} · ${o.name}\nTọa độ: (${o.x}, ${o.y}) km\nKhung giờ: ${fmtTime(o.start)}–${fmtTime(o.end)} (PV ${o.service}p, ${o.weight}kg)\n${
        info ? `Shipper ${info.c + 1} — Điểm dừng thứ ${info.stopIdx + 1}` : 'Chưa phân công'
      }`;

      svgBody += `
        <g class="svg-node" data-map-order="${i}">
          <circle class="main-node" cx="${p.x.toFixed(1)}" cy="${p.y.toFixed(1)}" r="15" fill="${fill}" stroke="var(--surface)" stroke-width="2.2"></circle>
          <text x="${p.x.toFixed(1)}" y="${(p.y + 4).toFixed(1)}" text-anchor="middle" font-size="10" font-weight="700" fill="#fff">${esc(o.id)}</text>
          ${info ? `
            <circle cx="${(p.x + 13).toFixed(1)}" cy="${(p.y - 12).toFixed(1)}" r="8.5" fill="var(--surface)" stroke="${fill}" stroke-width="1.8"></circle>
            <text x="${(p.x + 13).toFixed(1)}" y="${(p.y - 8.8).toFixed(1)}" text-anchor="middle" font-size="9" font-weight="700" fill="${fill}">${info.stopIdx + 1}</text>
          ` : ''}
          <text x="${p.x.toFixed(1)}" y="${(p.y + 27).toFixed(1)}" text-anchor="middle" font-size="10" font-weight="600"
            fill="var(--ink)" stroke="var(--surface)" stroke-width="3" paint-order="stroke">${esc(shortName)}</text>
          <title>${esc(tooltip)}</title>
        </g>
      `;
    });

    // Vẽ Kho trung tâm (0, 0)
    svgBody += `
      <g>
        <rect x="${(pts[0].x - 18).toFixed(1)}" y="${(pts[0].y - 18).toFixed(1)}" width="36" height="36" rx="9" fill="#1e293b" stroke="#fff" stroke-width="2.5"></rect>
        <text x="${pts[0].x.toFixed(1)}" y="${(pts[0].y + 4).toFixed(1)}" text-anchor="middle" font-size="10.5" font-weight="700" fill="#fff">KHO</text>
        <title>Kho xuất phát trung tâm (0, 0)</title>
      </g>
    `;

    // Vẽ vị trí xe Shipper khi đang bật chế độ mô phỏng theo phút
    if (simMinutes !== null && vehicles().length > 0) {
      $('simClockLabel').textContent = `🕒 ${fmtTime(simMinutes)}`;
      for (const v of visibleVehicles()) {
        const st = computeShipperStateAtMinute(v, simMinutes, pts);
        const col = colorOf(v.index);
        svgBody += `
          <g transform="translate(${st.x.toFixed(1)}, ${st.y.toFixed(1)})">
            <circle r="13" fill="${col}" fill-opacity="0.25"></circle>
            <rect x="-18" y="-11" width="36" height="22" rx="11" fill="${col}" stroke="#fff" stroke-width="2"></rect>
            <text x="0" y="4" text-anchor="middle" font-size="10" font-weight="700" fill="#fff">S${v.index + 1}</text>
            <text x="0" y="-16" text-anchor="middle" font-size="10" font-weight="700" fill="var(--ink)" stroke="var(--surface)" stroke-width="3" paint-order="stroke">${esc(st.label)}</text>
          </g>
        `;
      }
    } else {
      $('simClockLabel').textContent = 'Toàn tuyến';
    }

    const scaleFactor = zoom / 100;
    const tx = 460 * (1 - scaleFactor);
    const ty = 270 * (1 - scaleFactor);
    $('routeMapSvg').innerHTML = `${defs}<g transform="translate(${tx.toFixed(1)} ${ty.toFixed(1)}) scale(${scaleFactor})">${svgBody}</g>`;
    $('zoomLabel').textContent = `${zoom}%`;

    $('mapLegend').innerHTML = vehicles().map((v, i) => `
      <button type="button" class="pill-btn" data-shipper-filter="${i}" aria-pressed="${selectedShipper === null || selectedShipper === i}">
        <span class="color-dot" style="background:${colorOf(i)}"></span>
        Shipper ${i + 1}: ${v.seq.length} đơn · ${fmtNum(v.km)} km · ${v.load} kg (${fmtTime(v.t0)}–${fmtTime(v.ret)})
      </button>
    `).join('');
  }

  // ----------------------------- TAB 2: Đồ thị Xung đột & Tô màu -------------

  function conflictReasonText(code) {
    if (code === 1) return 'Vượt tải trọng tối đa';
    if (code === 3) return 'Vượt thời lượng ca làm việc';
    return 'Xung đột khung giờ / không kịp di chuyển';
  }

  function renderConflictGraph() {
    if (!result) {
      $('conflictGraphSvg').innerHTML = '';
      $('colorStepExplain').textContent = 'Hãy bấm "Tối ưu bằng C++" để xây dựng đồ thị xung đột và chạy 4 thuật toán tô màu.';
      $('colorStepsTableWrap').innerHTML = '';
      $('algCompareWrap').innerHTML = '';
      $('adjMatrixWrap').innerHTML = '';
      return;
    }

    const runIdx = Number($('colorAlgSelect').value || result.best);
    const run = result.runs[runIdx] || result.runs[result.best];
    const colorMode = $('graphColorSourceSelect').value; // 'raw' | 'final'
    const layoutMode = $('graphLayoutSelect').value;   // 'circle' | 'geo'

    const steps = run.steps || [];
    const totalSteps = steps.length;
    const curStep = colorMode === 'raw' ? Math.min(colorStep, totalSteps) : totalSteps;

    $('colorStepBadge').textContent = `${curStep} / ${totalSteps} bước`;
    $('colorStepPrev').disabled = colorMode !== 'raw' || curStep <= 0;
    $('colorStepFirst').disabled = colorMode !== 'raw' || curStep <= 0;
    $('colorStepNext').disabled = colorMode !== 'raw' || curStep >= totalSteps;
    $('colorStepLast').disabled = colorMode !== 'raw' || curStep >= totalSteps;
    $('colorStepPlay').disabled = colorMode !== 'raw';

    const handShakeSum = result.theory.degrees.reduce((s, d) => s + d, 0);
    $('graphStatsText').textContent = `|V| = ${orders.length} đỉnh · |E| = ${result.edges.length} cạnh · ∑deg(v) = ${handShakeSum} = 2|E| · ${result.theory.bfs.components} thành phần liên thông`;

    // Xác định màu từng đỉnh ở bước curStep
    const vertexColor = new Array(orders.length).fill(-1);
    let activeStepObj = null;
    if (colorMode === 'raw') {
      for (let s = 0; s < curStep; ++s) {
        vertexColor[steps[s].v] = steps[s].color;
      }
      if (curStep > 0 && curStep <= totalSteps) {
        activeStepObj = steps[curStep - 1];
      }
    } else {
      vehicles().forEach((v, shipperIdx) => {
        v.seq.forEach((ordIdx) => { vertexColor[ordIdx] = shipperIdx; });
      });
    }

    // Hộp giải thích bước tô màu hiện tại
    if (colorMode === 'final') {
      $('colorStepExplain').innerHTML = `<b>Đang hiển thị nhóm Shipper cuối cùng (${vehicles().length} shipper):</b> Đây là phân hoạch sau khi C++ sửa lịch theo tổng tải/ca, gộp nhóm nhỏ và tối ưu hóa thứ tự giao. Chuyển về <i>"Màu tô ban đầu"</i> để xem chạy từng bước tô màu lý thuyết.`;
    } else if (curStep === 0) {
      $('colorStepExplain').innerHTML = `<b>Bước 0 / ${totalSteps} (${esc(run.name)}):</b> Chưa đỉnh nào được tô màu. Bấm <b>Bước tiếp ▶</b> hoặc <b>▶ Tự chạy</b> để quan sát C++ chọn đỉnh và gán màu hợp lệ nhỏ nhất.`;
    } else if (activeStepObj) {
      const ord = orders[activeStepObj.v];
      $('colorStepExplain').innerHTML = `<b>Bước ${activeStepObj.step} / ${totalSteps} (${esc(run.name)}) — Xét đỉnh #${ord.id} (${esc(ord.name)}):</b> ${esc(activeStepObj.reason)}.`;
    }

    // Xử lý đỉnh đang được người dùng nhấp kiểm tra xung đột
    const neighborReasons = new Map();
    if (inspectedVertex !== null && inspectedVertex < orders.length) {
      for (const [u, v, rCode] of (result.edgeReasons || [])) {
        if (u === inspectedVertex) neighborReasons.set(v, rCode);
        else if (v === inspectedVertex) neighborReasons.set(u, rCode);
      }
      const ord = orders[inspectedVertex];
      const conflictItems = Array.from(neighborReasons.entries()).map(([nb, code]) =>
        `<li><b>#${orders[nb].id} (${esc(orders[nb].name)})</b> — ${fmtTime(orders[nb].start)}–${fmtTime(orders[nb].end)}, ${orders[nb].weight}kg: <i>${conflictReasonText(code)}</i></li>`
      );
      $('vertexInspectBox').hidden = false;
      $('vertexInspectBox').innerHTML = `
        <div class="flex-between">
          <b>🔍 Đang kiểm tra đỉnh #${ord.id} · ${esc(ord.name)} (Bậc deg = ${neighborReasons.size})</b>
          <button type="button" class="btn btn-xs" id="closeVertexInspectBtn">Bỏ chọn</button>
        </div>
        ${conflictItems.length
          ? `<ul style="margin:6px 0 0;padding-left:20px">${conflictItems.join('')}</ul>`
          : `<p class="mt-6">Đỉnh cô lập (bậc 0): không xung đột cặp đôi với bất kỳ đơn hàng nào khác.</p>`}
      `;
    } else {
      $('vertexInspectBox').hidden = true;
    }

    // Vẽ SVG Đồ thị xung đột
    const pts = layoutMode === 'circle' ? computeCirclePositions(orders.length) : computeGeoPositions(orders);
    let svg = '';

    if ($('showConflictEdgesCheck').checked) {
      for (const [u, v] of result.edges) {
        const isHighlighted = inspectedVertex !== null && (u === inspectedVertex || v === inspectedVertex);
        const isDimmed = inspectedVertex !== null && !isHighlighted;
        svg += `<line x1="${pts[u].x.toFixed(1)}" y1="${pts[u].y.toFixed(1)}" x2="${pts[v].x.toFixed(1)}" y2="${pts[v].y.toFixed(1)}"
          stroke="${isHighlighted ? 'var(--red)' : 'var(--muted)'}"
          stroke-width="${isHighlighted ? 2.8 : 1.15}"
          stroke-opacity="${isHighlighted ? 0.95 : isDimmed ? 0.1 : 0.3}"/>`;
      }
    }

    orders.forEach((o, i) => {
      const colIdx = vertexColor[i];
      const fill = colIdx >= 0 ? colorOf(colIdx) : '#94a3b8';
      const isJustColored = activeStepObj && activeStepObj.v === i;
      const isInspected = inspectedVertex === i;
      const isNeighbor = neighborReasons.has(i);
      const dimmed = inspectedVertex !== null && !isInspected && !isNeighbor;

      svg += `
        <g class="svg-node" data-conflict-vertex="${i}" opacity="${dimmed ? 0.28 : 1}">
          ${isJustColored ? `<circle cx="${pts[i].x.toFixed(1)}" cy="${pts[i].y.toFixed(1)}" r="23" fill="none" stroke="${fill}" stroke-width="3" stroke-dasharray="4 3"></circle>` : ''}
          ${isInspected ? `<circle cx="${pts[i].x.toFixed(1)}" cy="${pts[i].y.toFixed(1)}" r="22" fill="none" stroke="var(--red)" stroke-width="3"></circle>` : ''}
          <circle class="main-node" cx="${pts[i].x.toFixed(1)}" cy="${pts[i].y.toFixed(1)}" r="16" fill="${fill}" stroke="var(--surface)" stroke-width="2.2"></circle>
          <text x="${pts[i].x.toFixed(1)}" y="${(pts[i].y + 4).toFixed(1)}" text-anchor="middle" font-size="10.5" font-weight="700" fill="#fff">${o.id}</text>
          <text x="${pts[i].x.toFixed(1)}" y="${(pts[i].y + 28).toFixed(1)}" text-anchor="middle" font-size="10"
            fill="var(--ink)" stroke="var(--surface)" stroke-width="3" paint-order="stroke">deg=${result.theory.degrees[i]}${colIdx >= 0 ? ` · M${colIdx + 1}` : ''}</text>
          <title>#${o.id} · ${esc(o.name)} | Bậc ${result.theory.degrees[i]} | ${colIdx >= 0 ? `Màu ${colIdx + 1}` : 'Chưa tô'}</title>
        </g>
      `;
    });
    $('conflictGraphSvg').innerHTML = svg;

    // Bảng nhật ký từng bước tô màu
    $('colorStepsTableWrap').innerHTML = `
      <table class="data-table">
        <thead>
          <tr>
            <th>Bước</th>
            <th>Đỉnh được chọn</th>
            <th>Bậc deg(v)</th>
            <th>Độ bão hòa sat(v)</th>
            <th>Màu gán</th>
            <th>Giải thích chi tiết từ C++</th>
          </tr>
        </thead>
        <tbody>
          ${steps.map((st) => {
            const ord = orders[st.v];
            const activeCls = (colorMode === 'raw' && st.step === curStep) ? 'row-highlight' : '';
            return `
              <tr class="${activeCls}" data-jump-color-step="${st.step}" style="cursor:pointer" title="Nhấp để xem trạng thái tại bước ${st.step}">
                <td class="mono">${st.step}</td>
                <td><b>#${ord.id}</b> · ${esc(ord.name)}</td>
                <td class="mono">${st.deg}</td>
                <td class="mono">${st.sat}</td>
                <td><span class="color-dot" style="background:${colorOf(st.color)}"></span> Màu ${st.color + 1}</td>
                <td>${esc(st.reason)}</td>
              </tr>
            `;
          }).join('')}
        </tbody>
      </table>
    `;

    // Bảng so sánh 4 thuật toán tô màu
    const best = bestRun();
    $('algCompareWrap').innerHTML = `
      <div class="table-scroll">
        <table class="data-table">
          <thead>
            <tr>
              <th>Thuật toán</th>
              <th>Số màu ban đầu</th>
              <th>Sau sửa lịch (repair)</th>
              <th>Shipper cuối (merge)</th>
              <th>Tổng quãng đường</th>
              <th>TG Tô màu</th>
              <th>TG Toàn quy trình</th>
              <th>Kiểm chứng độc lập</th>
            </tr>
          </thead>
          <tbody>
            ${result.runs.map((r, i) => `
              <tr class="${i === result.best ? 'row-highlight' : ''}">
                <td><b>${esc(r.name)}</b>${i === result.best ? ' ★ (Được chọn)' : ''}</td>
                <td class="mono">${r.raw}${r.optimal ? ' ✓ (Chứng minh χ(G))' : ''}</td>
                <td class="mono">${r.afterRepair}</td>
                <td class="mono"><b>${r.k}</b></td>
                <td class="mono">${fmtNum(r.km)} km</td>
                <td class="mono">${fmtNum(r.colorMs, 3)} ms</td>
                <td class="mono">${fmtNum(r.ms, 3)} ms</td>
                <td class="${r.valid ? 'text-good' : 'text-bad'}">${r.valid ? '✓ Hợp lệ' : '✕ Thiếu đơn / vi phạm'}</td>
              </tr>
            `).join('')}
          </tbody>
        </table>
      </div>
      <div class="callout-box" style="margin:10px 0 0">
        <b>Đối chiếu Phương án gốc v1 (chỉ tô màu bảo toàn, không hậu xử lý):</b> ${result.v1.k} shipper, ${fmtNum(result.v1.km)} km —
        <span class="${result.v1.ok ? 'text-good' : 'text-bad'}">${result.v1.ok ? 'Đạt ràng buộc tải/ca' : 'Vi phạm ràng buộc tải trọng hoặc ca làm'}</span>.<br>
        <b>Phương án cải tiến v2 (${esc(best.name)} + Sửa lịch + Gộp nhóm + Local Search):</b> <b>${best.k} shipper, ${fmtNum(best.km)} km</b> (Cận dưới lý thuyết: <b>${result.lb} shipper</b>).
      </div>
    `;

    // Ma trận kề A (hiển thị tối đa 40 đỉnh đầu để bảng mượt)
    const maxMat = Math.min(orders.length, 40);
    $('adjMatrixWrap').innerHTML = `
      ${orders.length > maxMat ? `<p class="hint-text" style="padding:8px 10px">Đang hiển thị 40/${orders.length} đỉnh đầu tiên trên bảng (JSON xuất ra vẫn chứa đầy đủ toàn bộ ma trận).</p>` : ''}
      <table class="matrix-table mono">
        <thead>
          <tr>
            <th>A[u][v]</th>
            ${orders.slice(0, maxMat).map((o) => `<th title="${esc(o.name)}">#${o.id}</th>`).join('')}
            <th>deg(v)</th>
          </tr>
        </thead>
        <tbody>
          ${orders.slice(0, maxMat).map((o, u) => `
            <tr>
              <th title="${esc(o.name)}">#${o.id}</th>
              ${result.theory.matrix[u].slice(0, maxMat).map((val) => `<td class="${val ? 'matrix-one' : ''}">${val}</td>`).join('')}
              <td><b>${result.theory.degrees[u]}</b></td>
            </tr>
          `).join('')}
        </tbody>
      </table>
    `;
  }

  // ----------------------------- TAB 3: Lịch trình & Gantt -------------------

  function renderScheduleAndGantt() {
    const list = visibleVehicles();
    if (!result || list.length === 0) {
      $('ganttContainer').innerHTML = '<p class="hint-text">Chưa có dữ liệu lịch trình để vẽ biểu đồ Gantt.</p>';
      $('routeCardsContainer').innerHTML = '';
      return;
    }

    const allVeh = vehicles();
    const minT = Math.floor(Math.min(...allVeh.map((v) => v.t0)) / 60) * 60;
    const maxT = Math.ceil(Math.max(...allVeh.map((v) => v.ret)) / 60) * 60;
    const span = Math.max(60, maxT - minT);

    const segHtml = (t1, t2, bg, text, tip) => {
      if (t2 <= t1) return '';
      const left = ((t1 - minT) / span) * 100;
      const width = ((t2 - t1) / span) * 100;
      return `<span class="gantt-seg" style="left:${left.toFixed(2)}%;width:${width.toFixed(2)}%;background:${bg}" title="${esc(tip)}">${t2 - t1 >= 14 ? esc(text) : ''}</span>`;
    };

    const tickStep = span > 720 ? 120 : 60;
    let ticks = '';
    for (let t = minT; t <= maxT; t += tickStep) {
      ticks += `<span class="mono" style="left:${(((t - minT) / span) * 100).toFixed(2)}%">${fmtTime(t)}</span>`;
    }

    const rows = list.map((v) => {
      const col = colorOf(v.index);
      let prev = v.t0;
      let bars = '';
      for (const s of v.stops) {
        const ord = orders[s.id];
        bars += segHtml(prev, s.arrive, '#64748b', '', `Di chuyển tới #${ord.id} (${ord.name}): ${fmtTime(prev)}–${fmtTime(s.arrive)}`);
        bars += segHtml(s.arrive, s.begin, '#d97706', 'Chờ', `Chờ mở cửa #${ord.id}: ${fmtTime(s.arrive)}–${fmtTime(s.begin)} (${s.wait} phút)`);
        bars += segHtml(s.begin, s.finish, col, `#${ord.id}`, `Phục vụ #${ord.id} (${ord.name}): ${fmtTime(s.begin)}–${fmtTime(s.finish)}`);
        prev = s.finish;
      }
      bars += segHtml(prev, v.ret, '#64748b', '', `Quay về Kho: ${fmtTime(prev)}–${fmtTime(v.ret)}`);

      const shiftLeft = ((v.t0 - minT) / span) * 100;
      const shiftW = ((v.ret - v.t0) / span) * 100;
      return `
        <div class="gantt-row">
          <div class="gantt-label"><span class="color-dot" style="background:${col}"></span> Shipper ${v.index + 1}</div>
          <div class="gantt-track">
            <span class="gantt-shift-box" style="left:${shiftLeft.toFixed(2)}%;width:${shiftW.toFixed(2)}%"></span>
            ${bars}
          </div>
        </div>
      `;
    }).join('');

    $('ganttContainer').innerHTML = `
      <div class="gantt-canvas">
        <div class="gantt-axis">${ticks}</div>
        ${rows}
      </div>
      <div class="gantt-legend">
        <span><b style="color:var(--accent)">■</b> Màu Shipper: Đang giao hàng</span>
        <span><b style="color:#64748b">■</b> Xám: Di chuyển trên đường</span>
        <span><b style="color:#d97706">■</b> Cam: Đợi tới giờ mở cửa sổ</span>
        <span><b>[---]</b> Khung nét đứt: Tổng thời gian ca (Xuất kho → Về kho)</span>
      </div>
    `;

    const cap = readConfig().cap;
    const shiftLim = readConfig().shift;
    $('routeCardsContainer').innerHTML = list.map((v) => {
      const col = colorOf(v.index);
      const pct = cap > 0 ? Math.min(100, (v.load / cap) * 100) : 100;
      const shiftDur = v.ret - v.t0;
      return `
        <article class="route-card">
          <div class="route-card-head">
            <h4><span class="color-dot" style="background:${col}"></span>Shipper ${v.index + 1} <span class="badge-pill">${v.seq.length} đơn</span></h4>
            <strong class="mono">${fmtNum(v.km)} km</strong>
          </div>
          <div class="progress-bar"><span style="width:${pct.toFixed(1)}%;background:${col}"></span></div>
          <div class="route-meta-row">
            <span>Tải trọng: <b>${v.load} / ${cap || '∞'} kg</b></span>
            <span>Xuất kho <b>${fmtTime(v.t0)}</b> · Về kho <b>${fmtTime(v.ret)}</b> · Ca <b>${shiftDur}/${shiftLim || '∞'} phút</b></span>
          </div>
          <div class="table-scroll">
            <table class="data-table">
              <thead>
                <tr>
                  <th>TT</th>
                  <th>Đơn &amp; Địa điểm</th>
                  <th>Cửa sổ nhận</th>
                  <th>Đến nơi</th>
                  <th>Chờ</th>
                  <th>Bắt đầu</th>
                  <th>Xong</th>
                </tr>
              </thead>
              <tbody>
                ${v.stops.map((s, idx) => {
                  const ord = orders[s.id];
                  return `
                    <tr>
                      <td class="mono">${idx + 1}</td>
                      <td><b>#${ord.id}</b> · ${esc(ord.name)}</td>
                      <td class="mono">${fmtTime(ord.start)}–${fmtTime(ord.end)}</td>
                      <td class="mono">${fmtTime(s.arrive)}</td>
                      <td class="mono">${s.wait}p</td>
                      <td class="mono"><b>${fmtTime(s.begin)}</b></td>
                      <td class="mono">${fmtTime(s.finish)}</td>
                    </tr>
                  `;
                }).join('')}
              </tbody>
            </table>
          </div>
        </article>
      `;
    }).join('');
  }

  // ----------------------------- TAB 4: Minh họa Lý thuyết Slide -------------

  function renderTheory() {
    populateTheoryVertexSelectors();
    if (!result) {
      $('theoryGraphSvg').innerHTML = '';
      $('theoryExplainBox').textContent = 'Hãy bấm "Tối ưu bằng C++" để xem minh họa các thuật toán trong slide bài giảng.';
      $('theoryTableWrap').innerHTML = '';
      return;
    }

    const kind = $('theoryAlgSelect').value;
    const th = result.theory;
    const isTraversal = (kind === 'bfs' || kind === 'dfs');
    const isTree = (kind === 'prim' || kind === 'kruskal');

    const pts = isTraversal
      ? computeCirclePositions(orders.length)
      : computeGeoPositions([{ x: 0, y: 0 }, ...orders]);

    const nodeTitle = (idx) => isTraversal
      ? `#${orders[idx].id} (${orders[idx].name})`
      : idx === 0 ? 'Kho (0, 0)' : `#${orders[idx - 1].id} (${orders[idx - 1].name})`;

    const nodeBadge = (idx) => isTraversal
      ? String(orders[idx].id)
      : idx === 0 ? 'KHO' : String(orders[idx - 1].id);

    let svg = '';
    let explain = '';
    let tableHtml = '';
    let totalSteps = 0;
    const visitedNodes = new Set();

    // Vẽ nền các cạnh của mạng đường (cho Prim, Kruskal, Dijkstra) và cho phép nhấp để Khóa/Mở đường
    if (!isTraversal) {
      for (const [u, v, w, isBlocked] of (th.roadEdges || [])) {
        const mx = (pts[u].x + pts[v].x) / 2;
        const my = (pts[u].y + pts[v].y) / 2;
        svg += `
          <g class="road-edge-hit" data-road-u="${u}" data-road-v="${v}">
            <line x1="${pts[u].x.toFixed(1)}" y1="${pts[u].y.toFixed(1)}" x2="${pts[v].x.toFixed(1)}" y2="${pts[v].y.toFixed(1)}"
              stroke="transparent" stroke-width="12"></line>
            <line x1="${pts[u].x.toFixed(1)}" y1="${pts[u].y.toFixed(1)}" x2="${pts[v].x.toFixed(1)}" y2="${pts[v].y.toFixed(1)}"
              stroke="${isBlocked ? 'var(--red)' : 'var(--muted)'}"
              stroke-width="${isBlocked ? 2.2 : 1.2}"
              stroke-opacity="${isBlocked ? 0.85 : 0.25}"
              ${isBlocked ? 'stroke-dasharray="5 4"' : ''}></line>
            ${isBlocked ? `<text x="${mx.toFixed(1)}" y="${(my + 4).toFixed(1)}" text-anchor="middle" font-size="12" font-weight="700" fill="var(--red)">✕</text>` : ''}
            <title>${esc(nodeTitle(u))} ↔ ${esc(nodeTitle(v))} (${fmtNum(w)} km) — Nhấp để ${isBlocked ? 'MỞ LẠI đường' : 'KHÓA đường (mô phỏng sự cố/kẹt xe)'}</title>
          </g>
        `;
      }
    }

    if (isTraversal) {
      const tr = th[kind];
      totalSteps = tr.order.length;
      const n = Math.min(theoryStep, totalSteps);
      tr.order.slice(0, n).forEach((v) => visitedNodes.add(v));

      for (const [u, v] of result.edges) {
        svg += `<line x1="${pts[u].x.toFixed(1)}" y1="${pts[u].y.toFixed(1)}" x2="${pts[v].x.toFixed(1)}" y2="${pts[v].y.toFixed(1)}" stroke="var(--muted)" stroke-width="1" stroke-opacity="0.18"/>`;
      }
      for (const v of tr.order.slice(0, n)) {
        if (tr.parent[v] >= 0) {
          svg += `<line x1="${pts[tr.parent[v]].x.toFixed(1)}" y1="${pts[tr.parent[v]].y.toFixed(1)}" x2="${pts[v].x.toFixed(1)}" y2="${pts[v].y.toFixed(1)}" stroke="${colorOf(tr.component[v])}" stroke-width="3.2"/>`;
        }
      }

      explain = `<b>${kind.toUpperCase()} trên Đồ thị Xung đột</b> (xuất phát từ ${esc(nodeTitle(tr.root))}): Phát hiện <b>${tr.components} thành phần liên thông</b> → tạo rừng khung gồm <b>${orders.length - tr.components} cạnh</b>. ${
        kind === 'bfs'
          ? 'Sử dụng hàng đợi FIFO để duyệt từng mức từ gốc (Slide Cây_SV2020, trang 47–50).'
          : 'Sử dụng đệ quy đi sâu tối đa rồi quay lui (Slide Cây_SV2020, trang 51–53).'
      }`;

      $('theoryTableTitle').textContent = `Thứ tự duyệt ${kind.toUpperCase()}, mức (level) và đỉnh cha trong rừng khung`;
      tableHtml = `
        <div class="table-scroll">
          <table class="data-table">
            <thead><tr><th>Bước thăm</th><th>Đỉnh (Đơn hàng)</th><th>Thành phần liên thông</th><th>Mức (Level)</th><th>Đỉnh cha</th></tr></thead>
            <tbody>
              ${tr.order.map((v, i) => `
                <tr class="${i < n ? (i === n - 1 ? 'row-highlight' : '') : ''}" style="opacity:${i < n ? 1 : 0.45}">
                  <td class="mono">${i + 1}</td>
                  <td><b>${esc(nodeTitle(v))}</b></td>
                  <td class="mono">TPLT #${tr.component[v] + 1}</td>
                  <td class="mono">${tr.level[v]}</td>
                  <td>${tr.parent[v] < 0 ? '— (Gốc TPLT)' : esc(nodeTitle(tr.parent[v]))}</td>
                </tr>
              `).join('')}
            </tbody>
          </table>
        </div>
      `;
    } else if (isTree) {
      const tree = th[kind];
      totalSteps = tree.edges.length;
      const n = Math.min(theoryStep, totalSteps);
      if (kind === 'prim') visitedNodes.add(tree.root);

      for (const [u, v, w] of tree.edges.slice(0, n)) {
        visitedNodes.add(u);
        visitedNodes.add(v);
        const mx = (pts[u].x + pts[v].x) / 2;
        const my = (pts[u].y + pts[v].y) / 2;
        svg += `<line pointer-events="none" x1="${pts[u].x.toFixed(1)}" y1="${pts[u].y.toFixed(1)}" x2="${pts[v].x.toFixed(1)}" y2="${pts[v].y.toFixed(1)}" stroke="var(--accent)" stroke-width="3.4"/>`;
        svg += `<text pointer-events="none" x="${mx.toFixed(1)}" y="${(my - 6).toFixed(1)}" text-anchor="middle" font-size="10" font-weight="700" fill="var(--accent)" stroke="var(--surface)" stroke-width="3" paint-order="stroke">${fmtNum(w, 1)}</text>`;
      }

      explain = `<b>${kind === 'prim'
        ? `Thuật toán Prim (Slide Cây_SV2020, trang 14–20 — gốc tại ${esc(nodeTitle(tree.root))})`
        : 'Thuật toán Kruskal + Disjoint Set Union (Slide Cây_SV2020, trang 8–13)'}:</b> Kết nạp được <b>${tree.edges.length} / ${orders.length} cạnh</b>, tổng trọng số = <b>${fmtNum(tree.total)} km</b>. ${
        tree.connected ? 'Đồ thị liên thông, tạo thành Cây khung nhỏ nhất (MST).' : '⚠️ Do nhiều đường bị khóa nên mạng đường bị mất liên thông (chỉ tạo được cây/rừng khung cục bộ)!'
      }`;

      $('theoryTableTitle').textContent = `Danh sách các cạnh được kết nạp vào Cây khung (${kind === 'prim' ? 'Prim' : 'Kruskal'})`;
      tableHtml = `
        <div class="table-scroll">
          <table class="data-table">
            <thead><tr><th>Bước chọn</th><th>Đỉnh đầu u</th><th>Đỉnh cuối v</th><th>Trọng số cạnh (km)</th></tr></thead>
            <tbody>
              ${tree.edges.map(([u, v, w], i) => `
                <tr class="${i === n - 1 ? 'row-highlight' : ''}" style="opacity:${i < n ? 1 : 0.45}">
                  <td class="mono">${i + 1}</td>
                  <td>${esc(nodeTitle(u))}</td>
                  <td>${esc(nodeTitle(v))}</td>
                  <td class="mono"><b>${fmtNum(w)} km</b></td>
                </tr>
              `).join('')}
            </tbody>
          </table>
        </div>
      `;
    } else {
      // DIJKSTRA (kèm Bảng nhãn từng bước chuẩn Slide Đồ thị(1) trang 70-73)
      const sp = th.dijkstra;
      const steps = sp.steps || [];
      totalSteps = steps.length;
      const n = Math.min(theoryStep, totalSteps);
      const target = Number($('theoryTargetSelect').value || 1);

      const curSnapshot = n > 0 ? steps[n - 1] : null;
      const settledSet = new Set(steps.slice(0, n).map((st) => st.u));
      settledSet.forEach((v) => visitedNodes.add(v));

      // Vẽ cây đường đi ngắn nhất tại bước n
      if (curSnapshot) {
        for (let v = 0; v < pts.length; ++v) {
          const p = curSnapshot.parent[v];
          if (p >= 0 && settledSet.has(v)) {
            svg += `<line pointer-events="none" x1="${pts[p].x.toFixed(1)}" y1="${pts[p].y.toFixed(1)}" x2="${pts[v].x.toFixed(1)}" y2="${pts[v].y.toFixed(1)}" stroke="var(--accent)" stroke-width="1.8" stroke-opacity="0.45"/>`;
          }
        }
        // Làm nổi bật các cạnh vừa được giảm nhãn (relax) ở bước hiện tại
        for (const rv of curSnapshot.relaxed) {
          const p = curSnapshot.parent[rv];
          if (p >= 0) {
            svg += `<line pointer-events="none" x1="${pts[p].x.toFixed(1)}" y1="${pts[p].y.toFixed(1)}" x2="${pts[rv].x.toFixed(1)}" y2="${pts[rv].y.toFixed(1)}" stroke="var(--green)" stroke-width="2.8" stroke-dasharray="6 4"/>`;
          }
        }
      }

      // Vẽ đường đi ngắn nhất truy vết từ gốc tới đỉnh đích nếu đã có đường
      const activeParent = curSnapshot ? curSnapshot.parent : sp.parent;
      const activeDist = curSnapshot ? curSnapshot.dist : sp.distance;
      let pathTrace = [];
      if (activeDist[target] !== null && activeDist[target] !== undefined) {
        let cur = target, guard = 0;
        while (cur >= 0 && guard++ <= pts.length) {
          pathTrace.push(cur);
          if (cur === sp.source) break;
          cur = activeParent[cur];
        }
        pathTrace.reverse();
        if (pathTrace[0] === sp.source) {
          for (let i = 1; i < pathTrace.length; ++i) {
            svg += `<g pointer-events="none">${directedSegmentSvg(pts[pathTrace[i - 1]], pts[pathTrace[i]], '#dc2626', 3.8)}</g>`;
          }
        } else {
          pathTrace = [];
        }
      }

      const pathStr = pathTrace.length
        ? `${pathTrace.map((v) => nodeBadge(v)).join(' → ')} (Tổng = ${fmtNum(activeDist[target])} km, qua ${pathTrace.length - 1} chặng)`
        : 'Chưa tìm thấy đường tới đích ở bước này (hoặc bị cô lập do khóa đường)';

      explain = `<b>Dijkstra từ ${esc(nodeTitle(sp.source))} đến ${esc(nodeTitle(target))}</b> (Slide Đồ thị 1, trang 70–73): ${
        curSnapshot
          ? `Bước ${n}/${totalSteps} chốt đỉnh <b>${esc(nodeTitle(curSnapshot.u))}</b> với nhãn nhỏ nhất <code>L = ${fmtNum(curSnapshot.lu)} km</code>, cập nhật giảm nhãn cho <b>${curSnapshot.relaxed.length}</b> đỉnh kề.`
          : 'Bước 0: Khởi tạo L(nguồn) = 0, các đỉnh khác = ∞.'
      }<br><b>Truy vết đường đi ngắn nhất hiện tại:</b> <code>${esc(pathStr)}</code>`;

      // Bảng nhãn từng bước chuẩn Slide: mỗi hàng là 1 bước chốt đỉnh, mỗi cột là [L(v), trước(v)]
      const maxCols = Math.min(pts.length, 16);
      $('theoryTableTitle').textContent = `Bảng cập nhật nhãn từng bước [L(v), trước(v)] chuẩn Slide Học phần`;
      const lockedSoFar = new Set();
      tableHtml = `
        ${pts.length > maxCols ? `<p class="hint-text mb-6">Hiển thị ${maxCols}/${pts.length} đỉnh đầu trong bảng nhãn từng bước.</p>` : ''}
        <div class="table-scroll">
          <table class="data-table mono">
            <thead>
              <tr>
                <th>Vòng lặp</th>
                <th>Đỉnh chốt u</th>
                ${Array.from({ length: maxCols }, (_, v) => `<th title="${esc(nodeTitle(v))}">${esc(nodeBadge(v))}</th>`).join('')}
              </tr>
            </thead>
            <tbody>
              ${steps.slice(0, n || totalSteps).map((st, rowIdx) => {
                lockedSoFar.add(st.u);
                const relaxedSet = new Set(st.relaxed);
                const isCurRow = (n > 0 && rowIdx === n - 1);
                return `
                  <tr class="${isCurRow ? 'row-highlight' : ''}">
                    <td>Bước ${rowIdx + 1}</td>
                    <td><b>${esc(nodeBadge(st.u))}</b> (${fmtNum(st.lu, 1)})</td>
                    ${Array.from({ length: maxCols }, (_, v) => {
                      const dVal = st.dist[v];
                      const pVal = st.parent[v];
                      const isJustLocked = (v === st.u);
                      const isRelaxed = relaxedSet.has(v);
                      const cellCls = isJustLocked ? 'cell-locked' : isRelaxed ? 'cell-relaxed' : '';
                      const text = dVal === null
                        ? '(∞, -)'
                        : `(${fmtNum(dVal, 1)}, ${pVal < 0 ? '-' : nodeBadge(pVal)})${isJustLocked ? '*' : ''}`;
                      return `<td class="${cellCls}">${esc(text)}</td>`;
                    }).join('')}
                  </tr>
                `;
              }).join('')}
            </tbody>
          </table>
        </div>
        <p class="hint-text mt-6">Ký hiệu <code>(L, tr)*</code>: đỉnh vừa được chốt cố định ở vòng lặp đó; ô màu xanh lá: nhãn <code>L(v)</code> vừa được cập nhật tốt hơn qua đỉnh chốt <code>u</code>.</p>
      `;
    }

    // Vẽ các đỉnh lên trên cùng
    pts.forEach((p, idx) => {
      const isVisited = visitedNodes.has(idx);
      const isRoot = !isTraversal && idx === 0;
      const fill = isVisited ? 'var(--accent)' : (isRoot ? '#1e293b' : '#8b95a9');
      svg += `
        <g class="svg-node">
          <circle class="main-node" cx="${p.x.toFixed(1)}" cy="${p.y.toFixed(1)}" r="${isRoot ? 17 : 15}" fill="${fill}" stroke="var(--surface)" stroke-width="2.2"></circle>
          <text x="${p.x.toFixed(1)}" y="${(p.y + 4).toFixed(1)}" text-anchor="middle" font-size="10" font-weight="700" fill="#fff">${esc(nodeBadge(idx))}</text>
          <title>${esc(nodeTitle(idx))}</title>
        </g>
      `;
    });

    $('theoryGraphSvg').innerHTML = svg;
    $('theoryExplainBox').innerHTML = explain;
    $('theoryTableWrap').innerHTML = tableHtml;

    const shownStep = Math.min(theoryStep, totalSteps);
    $('theoryStepBadge').textContent = `${shownStep} / ${totalSteps} bước`;
    $('theoryStepFirst').disabled = shownStep <= 0;
    $('theoryStepPrev').disabled = shownStep <= 0;
    $('theoryStepNext').disabled = shownStep >= totalSteps;
    $('theoryStepLast').disabled = shownStep >= totalSteps;
    $('theoryStepNext').dataset.total = String(totalSteps);
  }

  // ----------------------------- TAB 5: Quản lý Đơn hàng ---------------------

  function renderOrdersManager() {
    const assignMap = new Map();
    vehicles().forEach((v, c) => v.stops.forEach((s, k) => assignMap.set(s.id, { c, k, s })));

    const q = $('orderSearchInput').value.trim().toLowerCase().replace(/^#/, '');
    const stFilter = $('orderStatusFilter').value;
    const sortBy = $('orderSortSelect').value;

    const filtered = orders
      .map((o, idx) => ({ o, idx, a: assignMap.get(idx) }))
      .filter(({ o, a }) => {
        const matchQuery = !q || String(o.id).includes(q) || o.name.toLowerCase().includes(q);
        const matchStatus = stFilter === 'all' || (stFilter === 'assigned' && a) || (stFilter === 'unassigned' && !a);
        return matchQuery && matchStatus;
      });

    filtered.sort((x, y) => {
      if (sortBy === 'id') return x.o.id - y.o.id;
      if (sortBy === 'start') return x.o.start - y.o.start;
      if (sortBy === 'weight') return y.o.weight - x.o.weight;
      return x.idx - y.idx;
    });

    $('mgrCountLabel').textContent = `Hiển thị ${filtered.length} / ${orders.length} đơn`;
    $('mgrOrdersBody').innerHTML = filtered.map(({ o, idx, a }) => `
      <tr>
        <td class="mono"><b>#${o.id}</b></td>
        <td>${esc(o.name)}</td>
        <td class="mono">(${fmtNum(o.x, 1)}, ${fmtNum(o.y, 1)}) km</td>
        <td class="mono">${fmtTime(o.start)}–${fmtTime(o.end)}</td>
        <td class="mono">${o.service} phút</td>
        <td class="mono">${o.weight} kg</td>
        <td>${a
          ? `<span class="color-dot" style="background:${colorOf(a.c)}"></span> Shipper ${a.c + 1} (TT #${a.k + 1})`
          : `<span class="${result ? 'text-bad' : 'hint-text'}">${result ? 'Không giao kịp / Quá tải' : 'Chưa chạy C++'}</span>`
        }</td>
        <td class="mono">${a ? fmtTime(a.s.begin) : '—'}</td>
        <td>
          <button type="button" class="btn btn-xs" data-mgr-edit="${idx}">Sửa</button>
          <button type="button" class="btn btn-xs" style="color:var(--red)" data-mgr-del="${idx}">Xóa</button>
        </td>
      </tr>
    `).join('') || `<tr><td colspan="9" style="text-align:center;padding:28px" class="hint-text">Không có đơn hàng nào khớp bộ lọc.</td></tr>`;
  }

  // ----------------------------- Chuyển Tab & Hộp thoại ----------------------

  function switchTab(tabName) {
    if (!['map', 'graph', 'schedule', 'theory', 'orders'].includes(tabName)) tabName = 'map';
    activeTab = tabName;
    document.querySelectorAll('.nav-tab').forEach((btn) => {
      btn.classList.toggle('active', btn.dataset.tab === tabName);
    });
    document.querySelectorAll('.tab-panel').forEach((panel) => {
      panel.classList.toggle('active', panel.id === `panel-${tabName}`);
    });
    $('workspaceGrid').classList.toggle('full-width', tabName === 'orders');
    renderAll();
  }

  function openOrderModal(idx = -1, defaultX = 0, defaultY = 0) {
    if (idx < 0 && orders.length >= 200) {
      notify('Đã đạt giới hạn tối đa 200 đơn hàng.');
      return;
    }
    editIndex = idx;
    const nextId = orders.length ? Math.max(...orders.map((o) => o.id)) + 1 : 101;
    const ord = idx < 0
      ? { id: nextId, name: `Điểm giao ${nextId}`, x: defaultX, y: defaultY, start: 540, end: 630, service: 10, weight: 5 }
      : orders[idx];

    const form = $('orderForm');
    form.elements.namedItem('name').value = ord.name || `Đơn ${ord.id}`;
    form.elements.namedItem('id').value = ord.id;
    form.elements.namedItem('weight').value = ord.weight;
    form.elements.namedItem('x').value = ord.x;
    form.elements.namedItem('y').value = ord.y;
    form.elements.namedItem('start').value = fmtTime(ord.start);
    form.elements.namedItem('end').value = fmtTime(ord.end);
    form.elements.namedItem('service').value = ord.service;

    $('dialogTitle').textContent = idx < 0 ? 'Thêm đơn hàng mới' : `Chỉnh sửa đơn #${ord.id}`;
    $('dialogError').textContent = '';
    $('orderDialog').showModal();
  }

  // ----------------------------- CSV / JSON & Sinh ngẫu nhiên ----------------

  function downloadBlob(filename, content, mime) {
    const url = URL.createObjectURL(new Blob([content], { type: mime }));
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    a.remove();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
  }

  function buildCsvString(rows) {
    return '\ufeff' + rows.map((row) =>
      row.map((cell) => `"${String(cell ?? '').replaceAll('"', '""')}"`).join(',')
    ).join('\r\n');
  }

  function parseCsvString(text) {
    const rows = [];
    let row = [], field = '', inQuote = false;
    text = text.replace(/^\ufeff/, '');
    for (let i = 0; i < text.length; ++i) {
      const c = text[i];
      if (c === '"') {
        if (inQuote && text[i + 1] === '"') { field += '"'; ++i; }
        else inQuote = !inQuote;
      } else if (c === ',' && !inQuote) {
        row.push(field); field = '';
      } else if ((c === '\n' || c === '\r') && !inQuote) {
        if (c === '\r' && text[i + 1] === '\n') ++i;
        row.push(field);
        if (row.some((x) => x.trim() !== '')) rows.push(row);
        row = []; field = '';
      } else {
        field += c;
      }
    }
    if (inQuote) throw new Error('File CSV có dấu ngoặc kép chưa đóng.');
    row.push(field);
    if (row.some((x) => x.trim() !== '')) rows.push(row);
    return rows;
  }

  function generateSeededRandomOrders(n, seed) {
    const makeLcg = (s) => {
      let state = (BigInt(s) * 6364136223846793005n + 1442695040888963407n) & ((1n << 64n) - 1n);
      return (a, b) => {
        state = (state * 6364136223846793005n + 1442695040888963407n) & ((1n << 64n) - 1n);
        return a + Number((state >> 33n) % BigInt(b - a + 1));
      };
    };
    const r = makeLcg(seed), rw = makeLcg(seed + 777);
    const list = [];
    for (let i = 1; i <= n; ++i) {
      const st = r(480, 1080);
      const len = r(40, 100);
      list.push({
        id: i,
        name: `Điểm ngẫu nhiên ${String(i).padStart(2, '0')}`,
        x: r(-80, 120) / 10,
        y: r(-80, 120) / 10,
        start: st,
        end: st + len,
        service: r(5, 15),
        weight: rw(1, 10)
      });
    }
    return list;
  }

  // ----------------------------- Gắn kết Sự kiện UI --------------------------

  // Chuyển tab chính
  document.querySelectorAll('.nav-tab').forEach((btn) => {
    btn.addEventListener('click', () => {
      location.hash = btn.dataset.tab;
      switchTab(btn.dataset.tab);
    });
  });

  window.addEventListener('hashchange', () => {
    const h = location.hash.slice(1);
    if (h) switchTab(h);
  });

  // Đổi giao diện Sáng / Tối
  $('themeBtn').addEventListener('click', () => {
    const next = document.documentElement.dataset.theme === 'dark' ? 'light' : 'dark';
    document.documentElement.dataset.theme = next;
    try { localStorage.setItem('shipper_graph_theme', next); } catch {}
  });

  // Bộ mẫu & Ràng buộc
  $('solveBtn').addEventListener('click', solveWithCpp);
  $('presetSampleBtn').addEventListener('click', () => {
    orders = presets.sample.map((o) => ({ ...o }));
    writeConfigToInputs({ exact: 1, cap: 25, shift: 240, road: 1 });
    invalidateData(true);
    notify('Đã nạp bộ 14 đơn chuẩn.');
  });
  $('resetSampleBtn').addEventListener('click', () => $('presetSampleBtn').click());
  $('emptyLoadSampleBtn').addEventListener('click', () => $('presetSampleBtn').click());

  $('presetHcmcBtn').addEventListener('click', () => {
    if (!presets.hcmc.length) {
      notify('Chưa tải được mẫu TP.HCM từ C++, hãy kiểm tra máy chủ.');
      return;
    }
    orders = presets.hcmc.map((o) => ({ ...o }));
    writeConfigToInputs({ exact: 1, cap: 25, shift: 240, road: 1.25 });
    invalidateData(true);
    notify('Đã nạp 18 địa điểm giao thực tế tại TP.HCM (hệ số đường ×1.25).');
  });

  for (const id of ['capacityInput', 'shiftInput', 'roadInput', 'conflictModeSelect']) {
    $(id).addEventListener('change', () => invalidateData(false));
  }
  $('autoSolveCheck').addEventListener('change', () => {
    saveWorkspace();
    if ($('autoSolveCheck').checked && !result && orders.length) solveWithCpp();
  });

  $('clearOrdersBtn').addEventListener('click', () => {
    orders = [];
    invalidateData(true);
  });

  // Bảng nhanh bên trái
  $('quickOrdersBody').addEventListener('click', (e) => {
    const editBtn = e.target.closest('[data-quick-edit]');
    if (editBtn) { openOrderModal(Number(editBtn.dataset.quickEdit)); return; }
    const delBtn = e.target.closest('[data-quick-del]');
    if (delBtn) {
      orders.splice(Number(delBtn.dataset.quickDel), 1);
      invalidateData(true);
    }
  });

  // Lọc Shipper
  document.addEventListener('click', (e) => {
    const filterBtn = e.target.closest('[data-shipper-filter]');
    if (filterBtn) {
      const val = filterBtn.dataset.shipperFilter;
      selectedShipper = (val === 'all') ? null : (selectedShipper === Number(val) ? null : Number(val));
      renderAll();
    }
    if (e.target.id === 'closeVertexInspectBtn') {
      inspectedVertex = null;
      renderConflictGraph();
    }
  });

  // Bản đồ: Zoom, Thêm điểm & Kéo-thả đổi tọa độ đơn hàng
  $('zoomInBtn').addEventListener('click', () => { zoom = Math.min(180, zoom + 15); renderMap(); });
  $('zoomOutBtn').addEventListener('click', () => { zoom = Math.max(60, zoom - 15); renderMap(); });
  $('zoomResetBtn').addEventListener('click', () => { zoom = 100; renderMap(); });
  $('mapAddModeBtn').addEventListener('click', () => {
    mapAddMode = !mapAddMode;
    renderMap();
    if (mapAddMode) notify('Nhấp vào vị trí trống trên bản đồ để thêm đơn hàng tại tọa độ đó.');
  });

  function clientToMapCoords(svgEl, clientX, clientY) {
    if (!mapTransform) return { x: 0, y: 0 };
    const pt = new DOMPoint(clientX, clientY).matrixTransform(svgEl.getScreenCTM().inverse());
    const factor = zoom / 100;
    const rawX = (pt.x - 460 * (1 - factor)) / factor;
    const rawY = (pt.y - 270 * (1 - factor)) / factor;
    return {
      x: Math.round(mapTransform.fromX(rawX) * 10) / 10,
      y: Math.round(mapTransform.fromY(rawY) * 10) / 10
    };
  }

  $('routeMapSvg').addEventListener('pointerdown', (e) => {
    const nodeG = e.target.closest('[data-map-order]');
    if (!nodeG) return;
    const idx = Number(nodeG.dataset.mapOrder);
    dragState = { idx, startClientX: e.clientX, startClientY: e.clientY, moved: false };
    $('routeMapSvg').setPointerCapture(e.pointerId);
  });

  $('routeMapSvg').addEventListener('pointermove', (e) => {
    if (!dragState) return;
    if (Math.hypot(e.clientX - dragState.startClientX, e.clientY - dragState.startClientY) > 5) {
      dragState.moved = true;
    }
    if (dragState.moved) {
      const pos = clientToMapCoords($('routeMapSvg'), e.clientX, e.clientY);
      orders[dragState.idx].x = pos.x;
      orders[dragState.idx].y = pos.y;
      renderMap();
    }
  });

  $('routeMapSvg').addEventListener('pointerup', (e) => {
    if (dragState) {
      const st = dragState;
      dragState = null;
      if (st.moved) {
        notify(`Đã dời #${orders[st.idx].id} tới (${orders[st.idx].x}, ${orders[st.idx].y}) km.`);
        invalidateData(false);
      } else {
        openOrderModal(st.idx);
      }
      return;
    }
    if (mapAddMode) {
      const pos = clientToMapCoords($('routeMapSvg'), e.clientX, e.clientY);
      openOrderModal(-1, pos.x, pos.y);
    }
  });

  // Mô phỏng xe chạy theo thời gian trên bản đồ
  $('simTimeSlider').addEventListener('input', () => {
    simMinutes = Number($('simTimeSlider').value);
    renderMap();
  });

  $('simPlayBtn').addEventListener('click', () => {
    if (!vehicles().length) {
      notify('Hãy chạy tối ưu lịch trước khi mô phỏng xe chạy.');
      return;
    }
    if (simPlayTimer) {
      clearInterval(simPlayTimer);
      simPlayTimer = null;
      $('simPlayBtn').textContent = '▶ Mô phỏng xe chạy';
      return;
    }
    const minT = Number($('simTimeSlider').min);
    const maxT = Number($('simTimeSlider').max);
    if (simMinutes === null || simMinutes >= maxT) simMinutes = minT;
    $('simPlayBtn').textContent = '⏸ Tạm dừng';
    simPlayTimer = setInterval(() => {
      simMinutes += 3;
      if (simMinutes >= maxT) {
        simMinutes = maxT;
        clearInterval(simPlayTimer);
        simPlayTimer = null;
        $('simPlayBtn').textContent = '▶ Mô phỏng xe chạy';
      }
      $('simTimeSlider').value = String(simMinutes);
      renderMap();
    }, 65);
  });

  $('simResetBtn').addEventListener('click', () => {
    if (simPlayTimer) { clearInterval(simPlayTimer); simPlayTimer = null; }
    $('simPlayBtn').textContent = '▶ Mô phỏng xe chạy';
    simMinutes = null;
    renderMap();
  });

  // Đồ thị xung đột & Chạy từng bước Tô màu
  $('colorAlgSelect').addEventListener('change', () => { stopAllTimers(); colorStep = Infinity; renderConflictGraph(); });
  $('graphColorSourceSelect').addEventListener('change', () => { stopAllTimers(); renderConflictGraph(); });
  $('graphLayoutSelect').addEventListener('change', renderConflictGraph);
  $('showConflictEdgesCheck').addEventListener('change', renderConflictGraph);

  $('conflictGraphSvg').addEventListener('click', (e) => {
    const vNode = e.target.closest('[data-conflict-vertex]');
    if (!vNode) return;
    const v = Number(vNode.dataset.conflictVertex);
    inspectedVertex = (inspectedVertex === v) ? null : v;
    renderConflictGraph();
  });

  $('colorStepsTableWrap').addEventListener('click', (e) => {
    const tr = e.target.closest('[data-jump-color-step]');
    if (!tr) return;
    $('graphColorSourceSelect').value = 'raw';
    colorStep = Number(tr.dataset.jumpColorStep);
    renderConflictGraph();
  });

  $('colorStepFirst').addEventListener('click', () => { stopAllTimers(); colorStep = 0; renderConflictGraph(); });
  $('colorStepLast').addEventListener('click', () => { stopAllTimers(); colorStep = Infinity; renderConflictGraph(); });
  $('colorStepPrev').addEventListener('click', () => {
    stopAllTimers();
    const maxS = orders.length;
    colorStep = Math.max(0, Math.min(colorStep, maxS) - 1);
    renderConflictGraph();
  });
  $('colorStepNext').addEventListener('click', () => {
    stopAllTimers();
    const maxS = orders.length;
    colorStep = Math.min(maxS, (colorStep === Infinity ? maxS : colorStep) + 1);
    renderConflictGraph();
  });
  $('colorStepPlay').addEventListener('click', () => {
    if (colorPlayTimer) { stopAllTimers(); return; }
    $('graphColorSourceSelect').value = 'raw';
    const maxS = orders.length;
    if (colorStep >= maxS) colorStep = 0;
    renderConflictGraph();
    $('colorStepPlay').textContent = '⏸ Dừng';
    colorPlayTimer = setInterval(() => {
      colorStep += 1;
      renderConflictGraph();
      if (colorStep >= maxS) stopAllTimers();
    }, 650);
  });

  // Tab Lý thuyết Slide: chọn thuật toán, gốc, mạng đường & Khóa cạnh sự cố
  $('theoryAlgSelect').addEventListener('change', () => {
    stopAllTimers();
    theoryStep = Infinity;
    populateTheoryVertexSelectors();
    solveWithCpp();
  });
  $('theoryRootSelect').addEventListener('change', () => {
    stopAllTimers();
    theoryStep = Infinity;
    solveWithCpp();
  });
  $('roadGraphKSelect').addEventListener('change', () => {
    stopAllTimers();
    theoryStep = Infinity;
    solveWithCpp();
  });
  $('theoryTargetSelect').addEventListener('change', renderTheory);

  $('theoryGraphSvg').addEventListener('click', (e) => {
    const hit = e.target.closest('[data-road-u]');
    if (!hit) return;
    const u = Number(hit.dataset.roadU);
    const v = Number(hit.dataset.roadV);
    const a = Math.min(u, v), b = Math.max(u, v);
    const existIdx = blockedEdges.findIndex(([x, y]) => x === a && y === b);
    if (existIdx >= 0) {
      blockedEdges.splice(existIdx, 1);
      notify('Đã mở lại đoạn đường vừa chọn.');
    } else {
      blockedEdges.push([a, b]);
      notify('Đã khóa đoạn đường (mô phỏng kẹt xe / sự cố). C++ đang tính lại đường vòng!');
    }
    updateBlockedButton();
    solveWithCpp();
  });

  $('clearBlockedBtn').addEventListener('click', () => {
    blockedEdges = [];
    updateBlockedButton();
    solveWithCpp();
    notify('Đã mở lại toàn bộ các đoạn đường.');
  });

  $('theoryStepFirst').addEventListener('click', () => { stopAllTimers(); theoryStep = 0; renderTheory(); });
  $('theoryStepLast').addEventListener('click', () => { stopAllTimers(); theoryStep = Infinity; renderTheory(); });
  $('theoryStepPrev').addEventListener('click', () => {
    stopAllTimers();
    const total = Number($('theoryStepNext').dataset.total || 0);
    theoryStep = Math.max(0, Math.min(theoryStep, total) - 1);
    renderTheory();
  });
  $('theoryStepNext').addEventListener('click', () => {
    stopAllTimers();
    const total = Number($('theoryStepNext').dataset.total || 0);
    theoryStep = Math.min(total, (theoryStep === Infinity ? total : theoryStep) + 1);
    renderTheory();
  });
  $('theoryStepPlay').addEventListener('click', () => {
    if (theoryPlayTimer) { stopAllTimers(); return; }
    const total = Number($('theoryStepNext').dataset.total || 0);
    if (theoryStep >= total) theoryStep = 0;
    renderTheory();
    $('theoryStepPlay').textContent = '⏸ Dừng';
    theoryPlayTimer = setInterval(() => {
      theoryStep += 1;
      renderTheory();
      if (theoryStep >= total) stopAllTimers();
    }, 700);
  });

  // Quản lý đơn hàng & Hộp thoại Thêm/Sửa
  $('openAddOrderBtn').addEventListener('click', () => openOrderModal(-1));
  $('mgrAddBtn').addEventListener('click', () => openOrderModal(-1));
  $('closeDialogBtn').addEventListener('click', () => $('orderDialog').close());
  $('cancelDialogBtn').addEventListener('click', () => $('orderDialog').close());

  $('orderForm').addEventListener('submit', (e) => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const newOrd = {
      id: Number(fd.get('id')),
      name: String(fd.get('name') || '').trim() || `Đơn ${fd.get('id')}`,
      x: Number(fd.get('x')),
      y: Number(fd.get('y')),
      start: parseTimeMins(fd.get('start')),
      end: parseTimeMins(fd.get('end')),
      service: Number(fd.get('service')),
      weight: Number(fd.get('weight'))
    };
    const nextOrders = orders.slice();
    if (editIndex < 0) nextOrders.push(newOrd);
    else nextOrders[editIndex] = newOrd;

    try {
      validateOrdersAndConfig(nextOrders);
      const countChanged = editIndex < 0;
      orders = nextOrders;
      $('orderDialog').close();
      invalidateData(countChanged);
    } catch (err) {
      $('dialogError').textContent = err.message;
    }
  });

  $('orderSearchInput').addEventListener('input', renderOrdersManager);
  $('orderStatusFilter').addEventListener('change', renderOrdersManager);
  $('orderSortSelect').addEventListener('change', renderOrdersManager);

  $('mgrOrdersBody').addEventListener('click', (e) => {
    const editBtn = e.target.closest('[data-mgr-edit]');
    if (editBtn) { openOrderModal(Number(editBtn.dataset.mgrEdit)); return; }
    const delBtn = e.target.closest('[data-mgr-del]');
    if (delBtn) {
      orders.splice(Number(delBtn.dataset.mgrDel), 1);
      invalidateData(true);
    }
  });

  // Sinh ngẫu nhiên & Xuất / Nhập file
  $('randomGenBtn').addEventListener('click', () => {
    const n = Number($('randomNInput').value);
    const seed = Number($('randomSeedInput').value);
    if (!Number.isInteger(n) || n < 1 || n > 120 || !Number.isInteger(seed) || seed < 0) {
      notify('Số đơn phải từ 1 đến 120, Seed là số nguyên không âm.');
      return;
    }
    orders = generateSeededRandomOrders(n, seed);
    invalidateData(true);
    notify(`Đã sinh ngẫu nhiên ${n} đơn (seed ${seed}).`);
  });

  $('exportCsvBtn').addEventListener('click', () => {
    const rows = [
      ['id', 'ten', 'x', 'y', 'bat_dau', 'ket_thuc', 'phuc_vu', 'khoi_luong'],
      ...orders.map((o) => [o.id, o.name, o.x, o.y, o.start, o.end, o.service, o.weight])
    ];
    downloadBlob('danh-sach-don-hang.csv', buildCsvString(rows), 'text/csv;charset=utf-8');
  });
  $('mgrExportBtn').addEventListener('click', () => $('exportCsvBtn').click());

  $('exportReportBtn').addEventListener('click', () => {
    if (!result) {
      notify('Hãy bấm "Tối ưu bằng C++" trước khi xuất báo cáo.');
      return;
    }
    const assignMap = new Map();
    vehicles().forEach((v, c) => v.stops.forEach((s, k) => assignMap.set(s.id, { v, c, k, s })));
    const rows = [[
      'id', 'ten_dia_diem', 'shipper', 'thu_tu_giao', 'x_km', 'y_km',
      'mo_cua', 'dong_cua', 'phuc_vu_phut', 'khoi_luong_kg',
      'gio_den', 'cho_phut', 'bat_dau_giao', 'hoan_thanh', 'xuat_kho', 've_kho', 'quang_duong_tuyen_km', 'trang_thai'
    ]];
    orders.forEach((o, i) => {
      const a = assignMap.get(i);
      rows.push([
        o.id, o.name, a ? `Shipper ${a.c + 1}` : '', a ? a.k + 1 : '',
        o.x, o.y, fmtTime(o.start), fmtTime(o.end), o.service, o.weight,
        a ? fmtTime(a.s.arrive) : '', a ? a.s.wait : '',
        a ? fmtTime(a.s.begin) : '', a ? fmtTime(a.s.finish) : '',
        a ? fmtTime(a.v.t0) : '', a ? fmtTime(a.v.ret) : '',
        a ? a.v.km : '', a ? 'Hop le' : 'Khong the giao'
      ]);
    });
    downloadBlob('bao-cao-dieu-phoi-shipper.csv', buildCsvString(rows), 'text/csv;charset=utf-8');
  });

  $('importCsvBtn').addEventListener('click', () => $('csvFileInput').click());
  $('mgrImportBtn').addEventListener('click', () => $('csvFileInput').click());

  $('csvFileInput').addEventListener('change', async (e) => {
    const file = e.target.files[0];
    if (!file) return;
    try {
      const rows = parseCsvString(await file.text());
      if (rows.length < 2) throw new Error('File CSV chưa có dòng dữ liệu.');
      const imported = rows.slice(1).map((r, idx) => {
        if (![7, 8].includes(r.length)) throw new Error(`Dòng ${idx + 2}: Cần 7 hoặc 8 cột theo mẫu CSV.`);
        const id = Number(r[0]);
        return {
          id,
          name: String(r[1] || '').trim() || `Đơn ${id}`,
          x: Number(r[2]),
          y: Number(r[3]),
          start: Number(r[4]),
          end: Number(r[5]),
          service: Number(r[6]),
          weight: r.length === 8 ? Number(r[7]) : 1
        };
      });
      validateOrdersAndConfig(imported);
      orders = imported;
      invalidateData(true);
      notify(`Đã nhập thành công ${imported.length} đơn hàng (giữ nguyên tên địa điểm).`);
    } catch (err) {
      notify(err.message);
    } finally {
      e.target.value = '';
    }
  });

  $('saveJsonBtn').addEventListener('click', () => {
    downloadBlob(
      'phien-do-an-shipper.json',
      JSON.stringify({ version: 2, orders, config: readConfig(), blockedEdges, result }, null, 2),
      'application/json'
    );
  });

  $('loadJsonBtn').addEventListener('click', () => $('jsonFileInput').click());
  $('jsonFileInput').addEventListener('change', async (e) => {
    const file = e.target.files[0];
    if (!file) return;
    try {
      const data = JSON.parse(await file.text());
      if (!Array.isArray(data.orders) || !data.config) throw new Error('File phiên JSON không đúng định dạng.');
      const nextOrders = data.orders.map((o) => ({ ...o, name: String(o.name || `Đơn ${o.id}`) }));
      validateOrdersAndConfig(nextOrders, data.config);
      orders = nextOrders;
      blockedEdges = Array.isArray(data.blockedEdges) ? data.blockedEdges : [];
      writeConfigToInputs(data.config);
      invalidateData(false);
      notify('Đã nạp phiên JSON và tính lại bằng C++.');
    } catch (err) {
      notify(err.message);
    } finally {
      e.target.value = '';
    }
  });

  // ----------------------------- Khởi chạy ứng dụng --------------------------

  async function initApp() {
    try {
      const savedTheme = localStorage.getItem('shipper_graph_theme');
      if (savedTheme === 'dark' || savedTheme === 'light') {
        document.documentElement.dataset.theme = savedTheme;
      }
    } catch {}

    if (location.protocol === 'file:') {
      setConnectionStatus(false);
      $('alertsBox').innerHTML = `<div class="banner bad">Bạn đang mở trực tiếp file HTML từ ổ đĩa nên trình duyệt không gọi được C++. Hãy nhấp đúp <b>MO-GIAO-DIEN.cmd</b> để máy chủ C++ mở giao diện tại <code>http://127.0.0.1:8080/</code>.</div>`;
      return;
    }

    await loadPresetsFromCpp();
    const restored = loadWorkspace();
    if (!restored && presets.sample.length) {
      orders = presets.sample.map((o) => ({ ...o }));
    }

    // Xác định tab khởi đầu theo URL hoặc hash
    const pathMap = { '/tuyen-duong': 'schedule', '/don-hang': 'orders', '/do-thi': 'graph', '/ly-thuyet': 'theory' };
    const initialTab = location.hash.slice(1) || pathMap[location.pathname] || 'map';
    populateTheoryVertexSelectors();
    switchTab(initialTab);
    await solveWithCpp();
  }

  initApp();
})();
