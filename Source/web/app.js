// =============================================================================
// FILE: web/app.js
// MÔ TẢ: Vẽ Đồ thị xung đột và minh họa từng bước thuật toán Welsh-Powell.
//        Viết bằng vòng lặp for cơ bản để sinh viên đọc hiểu 100%.
// =============================================================================

const BANG_MAU_XE = [
  '#ef4444', // Màu 1 (Xe 1): Đỏ
  '#3b82f6', // Màu 2 (Xe 2): Xanh dương
  '#10b981', // Màu 3 (Xe 3): Xanh lá
  '#f59e0b', // Màu 4 (Xe 4): Cam
  '#8b5cf6', // Màu 5 (Xe 5): Tím
  '#06b6d4', // Màu 6 (Xe 6): Xanh ngọc
  '#ec4899'  // Màu 7 (Xe 7): Hồng
];

let danhSachBo = Array.isArray(window.DU_LIEU_TU_CPP) ? window.DU_LIEU_TU_CPP : [];
let boHienTai = null;

let trangThai = 'GRAPH_BUILT'; // 'GRAPH_BUILT' | 'COLORED' | 'STEP'
let cheDoXem = 'GRAPH';        // 'GRAPH' | 'MAP'
let buocHienTai = 0;
let boDemTuChay = null;
let toaDoDinh = [];

const canvas = document.getElementById('graphCanvas');
const ctx = canvas.getContext('2d');

// ---------------------- 1. KHỞI TẠO & CHỌN DỮ LIỆU --------------------------

function khoiTao() {
  const select = document.getElementById('datasetSelect');
  select.innerHTML = '';

  for (let i = 0; i < danhSachBo.length; i++) {
    const opt = document.createElement('option');
    opt.value = i;
    opt.textContent = danhSachBo[i].tenBo;
    select.appendChild(opt);
  }

  if (danhSachBo.length > 0) {
    chonBoDuLieu(0);
  }
}

function chonBoDuLieu(viTri) {
  dungTuChay();
  boHienTai = danhSachBo[viTri];
  if (!boHienTai) return;

  document.getElementById('capInput').value = boHienTai.taiToiDa;
  trangThai = 'GRAPH_BUILT';
  document.getElementById('stepBar').hidden = true;
  document.getElementById('statusBanner').textContent =
    'Đang mở "' + boHienTai.tenBo + '" (' + boHienTai.donHang.length +
    ' đỉnh, ' + boHienTai.soCanh + ' cạnh). Nhấn "3. Tô màu" hoặc "4. Chạy từng bước".';

  capNhatBaKhungKetQua();
  veManHinh();
}

// ------------------ 2. VẼ ĐỒ THỊ BẰNG VÒNG LẶP CƠ BẢN -----------------------

function layMauHienTai() {
  const n = boHienTai.donHang.length;
  const mau = [];
  for (let i = 0; i < n; i++) {
    mau.push(-1);
  }

  if (trangThai === 'COLORED') {
    for (let i = 0; i < n; i++) {
      mau[i] = boHienTai.mauCuaDinh[i];
    }
  } else if (trangThai === 'STEP') {
    for (let i = 0; i < buocHienTai && i < boHienTai.cacBuoc.length; i++) {
      const b = boHienTai.cacBuoc[i];
      mau[b.dinh] = b.mau;
    }
  }
  return mau;
}

function veManHinh() {
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  if (!boHienTai) return;

  if (cheDoXem === 'MAP') {
    veBanDoLoTrinh();
  } else {
    veDoThiXungDot();
  }
}

function veDoThiXungDot() {
  const dsDon = boHienTai.donHang;
  const dsCanh = boHienTai.canhXungDot;
  const n = dsDon.length;

  // Xếp đều n đỉnh trên một vòng tròn để dễ quan sát các cạnh nối
  const tamX = canvas.width / 2;
  const tamY = canvas.height / 2;
  const banKinh = Math.min(canvas.width, canvas.height) * 0.36;
  toaDoDinh = [];

  for (let i = 0; i < n; i++) {
    const goc = -Math.PI / 2 + (2 * Math.PI * i) / n;
    toaDoDinh.push({
      x: tamX + banKinh * Math.cos(goc),
      y: tamY + banKinh * Math.sin(goc)
    });
  }

  const mauDinh = layMauHienTai();
  let dinhDangXet = -1;
  if (trangThai === 'STEP' && buocHienTai > 0 && buocHienTai <= boHienTai.cacBuoc.length) {
    dinhDangXet = boHienTai.cacBuoc[buocHienTai - 1].dinh;
  }

  // Vẽ các đoạn thẳng biểu diễn CẠNH xung đột
  for (let i = 0; i < dsCanh.length; i++) {
    const u = dsCanh[i].u;
    const v = dsCanh[i].v;
    const dangSang = (u === dinhDangXet || v === dinhDangXet);

    ctx.beginPath();
    ctx.moveTo(toaDoDinh[u].x, toaDoDinh[u].y);
    ctx.lineTo(toaDoDinh[v].x, toaDoDinh[v].y);
    ctx.strokeStyle = dangSang ? '#f59e0b' : '#94a3b8';
    ctx.lineWidth = dangSang ? 2.8 : 1.6;
    ctx.stroke();
  }

  // Vẽ các hình tròn biểu diễn ĐỈNH đơn hàng
  for (let i = 0; i < n; i++) {
    const p = toaDoDinh[i];
    const m = mauDinh[i];
    const laDinhMoi = (i === dinhDangXet);

    if (laDinhMoi) {
      ctx.beginPath();
      ctx.arc(p.x, p.y, 28, 0, 2 * Math.PI);
      ctx.fillStyle = 'rgba(250, 204, 21, 0.45)';
      ctx.fill();
    }

    ctx.beginPath();
    ctx.arc(p.x, p.y, 20, 0, 2 * Math.PI);
    ctx.fillStyle = m >= 0 ? BANG_MAU_XE[m % BANG_MAU_XE.length] : '#64748b';
    ctx.fill();
    ctx.lineWidth = laDinhMoi ? 3.5 : 2;
    ctx.strokeStyle = laDinhMoi ? '#ca8a04' : '#ffffff';
    ctx.stroke();

    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 11px Segoe UI';
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(dsDon[i].maDon, p.x, p.y);

    ctx.fillStyle = '#334155';
    ctx.font = '11px Segoe UI';
    ctx.fillText('deg=' + dsDon[i].bac + ' | ' + dsDon[i].khungGio, p.x, p.y + 31);
  }
}

function veBanDoLoTrinh() {
  const dsDon = boHienTai.donHang;
  const n = dsDon.length;

  let minX = 0, maxX = 0, minY = 0, maxY = 0;
  for (let i = 0; i < n; i++) {
    if (dsDon[i].x < minX) minX = dsDon[i].x;
    if (dsDon[i].x > maxX) maxX = dsDon[i].x;
    if (dsDon[i].y < minY) minY = dsDon[i].y;
    if (dsDon[i].y > maxY) maxY = dsDon[i].y;
  }

  const le = 55;
  const rong = Math.max(maxX - minX, 1);
  const cao = Math.max(maxY - minY, 1);

  function doiToaDo(x, y) {
    return {
      x: le + ((x - minX) / rong) * (canvas.width - 2 * le),
      y: canvas.height - le - ((y - minY) / cao) * (canvas.height - 2 * le)
    };
  }

  const kho = doiToaDo(0, 0);
  toaDoDinh = [];
  for (let i = 0; i < n; i++) {
    toaDoDinh.push(doiToaDo(dsDon[i].x, dsDon[i].y));
  }

  const mauDinh = layMauHienTai();

  // Vẽ đường nối lộ trình của từng xe từ Kho (0, 0)
  if (trangThai === 'COLORED' || trangThai === 'STEP') {
    for (let s = 0; s < boHienTai.loTrinhXe.length; s++) {
      const xe = boHienTai.loTrinhXe[s];
      ctx.beginPath();
      ctx.moveTo(kho.x, kho.y);
      for (let k = 0; k < xe.dsDon.length; k++) {
        const idx = xe.dsDon[k];
        if (mauDinh[idx] >= 0) {
          ctx.lineTo(toaDoDinh[idx].x, toaDoDinh[idx].y);
        }
      }
      ctx.strokeStyle = BANG_MAU_XE[s % BANG_MAU_XE.length];
      ctx.lineWidth = 2.4;
      ctx.stroke();
    }
  }

  // Vẽ ô vuông KHO (0, 0)
  ctx.fillStyle = '#0f172a';
  ctx.fillRect(kho.x - 15, kho.y - 15, 30, 30);
  ctx.fillStyle = '#ffffff';
  ctx.font = 'bold 10px Segoe UI';
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillText('KHO', kho.x, kho.y);

  // Vẽ các đỉnh đơn hàng
  for (let i = 0; i < n; i++) {
    const p = toaDoDinh[i];
    const m = mauDinh[i];
    ctx.beginPath();
    ctx.arc(p.x, p.y, 17, 0, 2 * Math.PI);
    ctx.fillStyle = m >= 0 ? BANG_MAU_XE[m % BANG_MAU_XE.length] : '#64748b';
    ctx.fill();
    ctx.strokeStyle = '#ffffff';
    ctx.lineWidth = 2;
    ctx.stroke();

    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 10px Segoe UI';
    ctx.fillText(dsDon[i].maDon, p.x, p.y);
  }
}

// -------------------- 3. HIỂN THỊ 3 KHUNG KẾT QUẢ DƯỚI ----------------------

function capNhatBaKhungKetQua() {
  const dsDon = boHienTai.donHang;
  const dsCanh = boHienTai.canhXungDot;
  const n = dsDon.length;

  // Khung 1: Danh sách các cạnh xung đột
  document.getElementById('edgeCountLabel').textContent = dsCanh.length;
  const conflictBox = document.getElementById('conflictList');
  if (dsCanh.length === 0) {
    conflictBox.innerHTML = '<div class="item-card">Không có cặp đơn nào xung đột.</div>';
  } else {
    let htmlCanh = '';
    for (let i = 0; i < dsCanh.length; i++) {
      const c = dsCanh[i];
      htmlCanh += '<div class="item-card conflict">' +
        '<strong>#' + (i + 1) + ': ' + dsDon[c.u].maDon + ' ↔ ' + dsDon[c.v].maDon + '</strong><br/>' +
        '<span>' + c.lyDo + '</span></div>';
    }
    conflictBox.innerHTML = htmlCanh;
  }

  // Khung 2: Thông tin Lý thuyết Đồ thị đã học (Thực hành 1 & 2) + Số màu
  let dsCoLap = [];
  let dsTreo = [];
  for (let i = 0; i < n; i++) {
    if (dsDon[i].coLap) dsCoLap.push(dsDon[i].maDon);
    if (dsDon[i].dinhTreo) dsTreo.push(dsDon[i].maDon);
  }

  let htmlTomTat = '';
  if (trangThai === 'COLORED' || trangThai === 'STEP') {
    htmlTomTat += '<div class="big-result">' +
      '<div>Kết quả Welsh–Powell — Số lượng Xe tối thiểu:</div>' +
      '<div class="big-number">' + boHienTai.soMau + ' Xe</div></div>';
  }

  htmlTomTat += '<table class="compare-table"><tbody>' +
    '<tr><th>Đơn đồ thị vô hướng</th><td>' + (boHienTai.laDonDoThiVoHuong ? 'Hợp lệ (không khuyên, đối xứng)' : 'Không') + '</td></tr>' +
    '<tr><th>Số đỉnh / Số cạnh</th><td>' + n + ' đỉnh, ' + boHienTai.soCanh + ' cạnh</td></tr>' +
    '<tr><th>Số miền liên thông</th><td>' + boHienTai.soMienLienThong + ' miền (hàm visit)</td></tr>' +
    '<tr><th>Đỉnh cô lập (deg = 0)</th><td>' + (dsCoLap.length > 0 ? dsCoLap.join(', ') : 'Không có') + '</td></tr>' +
    '<tr><th>Đỉnh treo (deg = 1)</th><td>' + (dsTreo.length > 0 ? dsTreo.join(', ') : 'Không có') + '</td></tr>' +
    '</tbody></table>';
  document.getElementById('summaryBox').innerHTML = htmlTomTat;

  // Khung 3: Chi tiết từng Xe
  const vehicleBox = document.getElementById('vehicleList');
  if (trangThai !== 'COLORED' && trangThai !== 'STEP') {
    vehicleBox.innerHTML = '<div class="empty-hint">Nhấn "3. Tô màu (Welsh–Powell)" để phân bổ đơn vào các xe.</div>';
  } else {
    let htmlXe = '';
    for (let i = 0; i < boHienTai.loTrinhXe.length; i++) {
      const xe = boHienTai.loTrinhXe[i];
      const mau = BANG_MAU_XE[i % BANG_MAU_XE.length];
      let tenCacDon = [];
      for (let k = 0; k < xe.dsDon.length; k++) {
        tenCacDon.push(dsDon[xe.dsDon[k]].maDon);
      }
      htmlXe += '<div class="item-card" style="border-left-color:' + mau + '">' +
        '<strong style="color:' + mau + '">Xe ' + xe.maXe + ' (' + xe.dsDon.length + ' đơn | ' +
        xe.tongKg + '/' + boHienTai.taiToiDa + ' kg | ' + xe.tongKm.toFixed(2) + ' km)</strong><br/>' +
        '<span>Lộ trình: Kho → ' + tenCacDon.join(' → ') + ' → Kho</span></div>';
    }
    vehicleBox.innerHTML = htmlXe;
  }
}

// --------------- 4. TÍNH LẠI KHI ĐỔI TẢI TRỌNG HOẶC TẢI CSV -----------------

function tinhLaiTrenTrinhDuyet(tenBo, dsDonGoc, taiToiDa) {
  const n = dsDonGoc.length;
  const G = [];
  for (let i = 0; i < n; i++) {
    const hang = [];
    for (let j = 0; j < n; j++) hang.push(0);
    G.push(hang);
  }

  function kc(x1, y1, x2, y2) {
    return Math.sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)) * 1.2;
  }

  const canhXungDot = [];
  for (let i = 0; i < n; i++) {
    for (let j = i + 1; j < n; j++) {
      const a = dsDonGoc[i], b = dsDonGoc[j];
      const phutDi = Math.ceil(kc(a.x, a.y, b.x, b.y) / 0.5);
      const kipAB = (a.gioMo + a.phutGiao + phutDi <= b.gioDong);
      const kipBA = (b.gioMo + b.phutGiao + phutDi <= a.gioDong);

      if (taiToiDa > 0 && a.khoiLuong + b.khoiLuong > taiToiDa) {
        G[i][j] = G[j][i] = 1;
        canhXungDot.push({ u: i, v: j, lyDo: 'Quá tải trọng xe (' + (a.khoiLuong + b.khoiLuong) + 'kg > ' + taiToiDa + 'kg)' });
      } else if (!kipAB && !kipBA) {
        G[i][j] = G[j][i] = 1;
        canhXungDot.push({ u: i, v: j, lyDo: 'Trùng khung giờ + cách ' + phutDi + ' phút chạy xe' });
      }
    }
  }

  // Xét thành phần liên thông bằng hàm visit giống C++
  const visited = [];
  for (let i = 0; i < n; i++) visited.push(0);
  function visit(u, nhan) {
    visited[u] = nhan;
    for (let v = 0; v < n; v++) {
      if (G[u][v] !== 0 && visited[v] === 0) visit(v, nhan);
    }
  }
  let soMien = 0;
  for (let i = 0; i < n; i++) {
    if (visited[i] === 0) {
      soMien++;
      visit(i, soMien);
    }
  }

  function doiGio(p) {
    const h = Math.floor(p / 60);
    const m = p % 60;
    return (h < 10 ? '0' : '') + h + ':' + (m < 10 ? '0' : '') + m;
  }

  const donHang = [];
  for (let i = 0; i < n; i++) {
    let bac = 0;
    for (let j = 0; j < n; j++) if (G[i][j] === 1) bac++;
    const d = dsDonGoc[i];
    donHang.push({
      maDon: d.maDon, diaChi: d.diaChi, x: d.x, y: d.y,
      gioMo: d.gioMo, gioDong: d.gioDong,
      khungGio: doiGio(d.gioMo) + ' - ' + doiGio(d.gioDong),
      phutGiao: d.phutGiao, khoiLuong: d.khoiLuong,
      bac: bac, coLap: bac === 0, dinhTreo: bac === 1,
      mienLienThong: visited[i],
      kcTuKho: Number(kc(0, 0, d.x, d.y).toFixed(2))
    });
  }

  // Thuật toán Welsh-Powell giống hệt trong src/Algorithms.cpp
  const thuTu = [];
  for (let i = 0; i < n; i++) thuTu.push(i);
  for (let i = 0; i < n - 1; i++) {
    let vtMax = i;
    for (let j = i + 1; j < n; j++) {
      if (donHang[thuTu[j]].bac > donHang[thuTu[vtMax]].bac) vtMax = j;
    }
    const tam = thuTu[i]; thuTu[i] = thuTu[vtMax]; thuTu[vtMax] = tam;
  }

  const mauCuaDinh = [];
  for (let i = 0; i < n; i++) mauCuaDinh.push(-1);
  const cacBuoc = [];
  let mauHienTai = 0, daTo = 0;

  while (daTo < n) {
    const nhom = [];
    let tai = 0;
    for (let i = 0; i < n; i++) {
      const u = thuTu[i];
      if (mauCuaDinh[u] !== -1) continue;
      let biKe = false;
      for (let k = 0; k < nhom.length; k++) {
        if (G[u][nhom[k]] === 1) biKe = true;
      }
      const vuotTai = (nhom.length > 0 && taiToiDa > 0 && tai + donHang[u].khoiLuong > taiToiDa);
      if (!biKe && !vuotTai) {
        mauCuaDinh[u] = mauHienTai;
        nhom.push(u);
        tai += donHang[u].khoiLuong;
        daTo++;
        cacBuoc.push({
          buoc: cacBuoc.length + 1, dinh: u, mau: mauHienTai, bac: donHang[u].bac,
          giaiThich: 'Xét ' + donHang[u].maDon + ' (deg = ' + donHang[u].bac + '): hợp lệ, tổng tải ' + tai + 'kg → Tô Màu ' + (mauHienTai + 1) + ' (Xe ' + (mauHienTai + 1) + ')'
        });
      }
    }
    mauHienTai++;
  }

  const loTrinhXe = [];
  for (let m = 0; m < mauHienTai; m++) {
    const ds = [];
    let tongKg = 0;
    for (let i = 0; i < n; i++) {
      if (mauCuaDinh[i] === m) {
        ds.push(i);
        tongKg += donHang[i].khoiLuong;
      }
    }
    for (let i = 0; i < ds.length - 1; i++) {
      for (let j = i + 1; j < ds.length; j++) {
        if (donHang[ds[j]].gioMo < donHang[ds[i]].gioMo) {
          const t = ds[i]; ds[i] = ds[j]; ds[j] = t;
        }
      }
    }
    let cx = 0, cy = 0, tongKm = 0;
    for (let k = 0; k < ds.length; k++) {
      const d = donHang[ds[k]];
      tongKm += kc(cx, cy, d.x, d.y);
      cx = d.x; cy = d.y;
    }
    if (ds.length > 0) tongKm += kc(cx, cy, 0, 0);
    loTrinhXe.push({ maXe: m + 1, tongKg: tongKg, tongKm: Number(tongKm.toFixed(2)), dsDon: ds });
  }

  return {
    tenBo: tenBo, taiToiDa: taiToiDa, laDonDoThiVoHuong: true,
    soCanh: canhXungDot.length, soMienLienThong: soMien, soMau: mauHienTai,
    donHang: donHang, canhXungDot: canhXungDot,
    mauCuaDinh: mauCuaDinh, cacBuoc: cacBuoc, loTrinhXe: loTrinhXe
  };
}

// --------------------- 5. SỰ KIỆN NÚT BẤM TRÊN GIAO DIỆN --------------------

function dungTuChay() {
  if (boDemTuChay) {
    clearInterval(boDemTuChay);
    boDemTuChay = null;
    document.getElementById('btnAutoPlay').textContent = '▶ Tự chạy';
  }
}

function capNhatThanhStep() {
  const tong = boHienTai.cacBuoc.length;
  document.getElementById('stepCounter').textContent = 'Bước ' + buocHienTai + ' / ' + tong;
  document.getElementById('stepExplain').textContent = buocHienTai === 0
    ? 'Chưa tô đỉnh nào. Nhấn "Bước tiếp (NEXT) ▶" để chạy bước 1.'
    : boHienTai.cacBuoc[buocHienTai - 1].giaiThich;
  veManHinh();
}

document.getElementById('datasetSelect').addEventListener('change', function (e) {
  chonBoDuLieu(Number(e.target.value));
});

document.getElementById('capInput').addEventListener('change', function (e) {
  const taiMoi = Math.max(0, Number(e.target.value) || 0);
  boHienTai = tinhLaiTrenTrinhDuyet(boHienTai.tenBo, boHienTai.donHang, taiMoi);
  capNhatBaKhungKetQua();
  veManHinh();
});

document.getElementById('btnBuildGraph').addEventListener('click', function () {
  dungTuChay();
  trangThai = 'GRAPH_BUILT';
  document.getElementById('stepBar').hidden = true;
  document.getElementById('statusBanner').textContent =
    'Đã dựng Đơn đồ thị vô hướng: ' + boHienTai.donHang.length + ' đỉnh, ' +
    boHienTai.soCanh + ' cạnh, ' + boHienTai.soMienLienThong + ' miền liên thông.';
  capNhatBaKhungKetQua();
  veManHinh();
});

document.getElementById('btnRunColor').addEventListener('click', function () {
  dungTuChay();
  trangThai = 'COLORED';
  document.getElementById('stepBar').hidden = true;
  document.getElementById('statusBanner').textContent =
    'Đã chạy Welsh–Powell: Cần tối thiểu ' + boHienTai.soMau + ' Xe cho ' + boHienTai.donHang.length + ' đơn hàng.';
  capNhatBaKhungKetQua();
  veManHinh();
});

document.getElementById('btnStepMode').addEventListener('click', function () {
  dungTuChay();
  trangThai = 'STEP';
  buocHienTai = 0;
  document.getElementById('stepBar').hidden = false;
  document.getElementById('statusBanner').textContent = 'Đang chạy từng bước thuật toán Welsh–Powell.';
  capNhatBaKhungKetQua();
  capNhatThanhStep();
});

document.getElementById('btnNextStep').addEventListener('click', function () {
  if (buocHienTai < boHienTai.cacBuoc.length) {
    buocHienTai++;
    capNhatThanhStep();
  }
});

document.getElementById('btnPrevStep').addEventListener('click', function () {
  if (buocHienTai > 0) {
    buocHienTai--;
    capNhatThanhStep();
  }
});

document.getElementById('btnAutoPlay').addEventListener('click', function () {
  if (boDemTuChay) {
    dungTuChay();
    return;
  }
  if (buocHienTai >= boHienTai.cacBuoc.length) buocHienTai = 0;
  document.getElementById('btnAutoPlay').textContent = '⏸ Tạm dừng';
  boDemTuChay = setInterval(function () {
    if (buocHienTai < boHienTai.cacBuoc.length) {
      buocHienTai++;
      capNhatThanhStep();
    } else {
      dungTuChay();
    }
  }, 850);
});

document.getElementById('btnViewGraph').addEventListener('click', function () {
  cheDoXem = 'GRAPH';
  document.getElementById('btnViewGraph').classList.add('active');
  document.getElementById('btnViewMap').classList.remove('active');
  veManHinh();
});

document.getElementById('btnViewMap').addEventListener('click', function () {
  cheDoXem = 'MAP';
  document.getElementById('btnViewMap').classList.add('active');
  document.getElementById('btnViewGraph').classList.remove('active');
  veManHinh();
});

// Tách dòng CSV bằng vòng lặp for giống hệt hàm tachDongCsv trong C++
function tachDongCsvJs(dong) {
  const cot = [];
  let hienTai = '';
  let trongNgoac = false;
  for (let i = 0; i < dong.length; i++) {
    const ch = dong[i];
    if (ch === '"') trongNgoac = !trongNgoac;
    else if (ch === ',' && !trongNgoac) {
      cot.push(hienTai.trim());
      hienTai = '';
    } else if (ch !== '\r') hienTai += ch;
  }
  cot.push(hienTai.trim());
  return cot;
}

document.getElementById('csvFileInput').addEventListener('change', function (e) {
  const file = e.target.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = function () {
    const cacDong = String(reader.result).split('\n');
    const dsDon = [];
    for (let i = 1; i < cacDong.length; i++) {
      if (cacDong[i].trim() === '') continue;
      const cot = tachDongCsvJs(cacDong[i]);
      if (cot.length < 8) continue;
      const doiPhut = function (s) {
        if (s.indexOf(':') !== -1) {
          const p = s.split(':');
          return Number(p[0]) * 60 + Number(p[1]);
        }
        return Number(s);
      };
      dsDon.push({
        maDon: cot[0], diaChi: cot[1],
        x: Number(cot[2]), y: Number(cot[3]),
        gioMo: doiPhut(cot[4]), gioDong: doiPhut(cot[5]),
        phutGiao: Number(cot[6]), khoiLuong: Number(cot[7])
      });
    }
    if (dsDon.length > 0) {
      const tai = Math.max(0, Number(document.getElementById('capInput').value) || 25);
      danhSachBo.unshift(tinhLaiTrenTrinhDuyet('File tải lên: ' + file.name, dsDon, tai));
      khoiTao();
    }
  };
  reader.readAsText(file, 'UTF-8');
});

document.getElementById('btnExportCsv').addEventListener('click', function () {
  if (!boHienTai) return;
  const dsDon = boHienTai.donHang;
  let csv = '\uFEFFBANG TONG HOP PHAN BO XE (WELSH-POWELL)\n';
  csv += 'Xe,So don,Tong khoi luong (kg),Quang duong (km),Thu tu giao tu Kho\n';

  for (let i = 0; i < boHienTai.loTrinhXe.length; i++) {
    const xe = boHienTai.loTrinhXe[i];
    let ten = [];
    for (let k = 0; k < xe.dsDon.length; k++) ten.push(dsDon[xe.dsDon[k]].maDon);
    csv += 'Xe ' + xe.maXe + ',' + xe.dsDon.length + ',' + xe.tongKg + ',' +
           xe.tongKm + ',"Kho -> ' + ten.join(' -> ') + ' -> Kho"\n';
  }

  csv += '\nDANH SACH DON HANG CHI TIET\nMa don,Dia chi,Khung gio,Khoi luong (kg),Xe duoc phan\n';
  for (let i = 0; i < dsDon.length; i++) {
    csv += dsDon[i].maDon + ',"' + dsDon[i].diaChi + '",' + dsDon[i].khungGio + ',' +
           dsDon[i].khoiLuong + ',Xe ' + (boHienTai.mauCuaDinh[i] + 1) + '\n';
  }

  const blob = new Blob([csv], { type: 'text/csv;charset=utf-8;' });
  const link = document.createElement('a');
  link.href = URL.createObjectURL(blob);
  link.download = 'Ket_Qua_Phan_Bo_Welsh_Powell.csv';
  link.click();
});

canvas.addEventListener('mousemove', function (e) {
  if (!boHienTai || toaDoDinh.length === 0) return;
  const rect = canvas.getBoundingClientRect();
  const mx = ((e.clientX - rect.left) / rect.width) * canvas.width;
  const my = ((e.clientY - rect.top) / rect.height) * canvas.height;

  const tooltip = document.getElementById('hoverTooltip');
  const mauDinh = layMauHienTai();

  for (let i = 0; i < toaDoDinh.length; i++) {
    const dx = mx - toaDoDinh[i].x;
    const dy = my - toaDoDinh[i].y;
    if (Math.sqrt(dx * dx + dy * dy) <= 22) {
      const d = boHienTai.donHang[i];
      const tenXe = mauDinh[i] >= 0 ? ('Xe ' + (mauDinh[i] + 1)) : 'Chưa phân xe';
      tooltip.hidden = false;
      tooltip.style.left = Math.min(e.offsetX + 15, rect.width - 260) + 'px';
      tooltip.style.top = Math.max(e.offsetY - 10, 10) + 'px';
      tooltip.innerHTML =
        '<strong>📦 ' + d.maDon + ' — ' + tenXe + '</strong><br/>' +
        '📍 ' + d.diaChi + '<br/>' +
        '⏰ Khung giờ: ' + d.khungGio + '<br/>' +
        '⚖️ Nặng: ' + d.khoiLuong + ' kg | Bậc deg = ' + d.bac + '<br/>' +
        '🔗 Miền liên thông thứ ' + d.mienLienThong + ' | Cách Kho: ' + d.kcTuKho + ' km';
      return;
    }
  }
  tooltip.hidden = true;
});

canvas.addEventListener('mouseleave', function () {
  document.getElementById('hoverTooltip').hidden = true;
});

khoiTao();
