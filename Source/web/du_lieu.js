// File nay duoc sinh TU DONG boi chuong trinh C++ (src/main.cpp)
window.DU_LIEU_TU_CPP = [
  {
    "tenBo": "Bộ 1: 10 đơn giao hàng thực tế tại TP.HCM",
    "taiToiDa": 25,
    "soCanh": 10,
    "donHang": [
      {"maDon": "DH01", "diaChi": "ĐH Sư Phạm CS1 - 280 An Dương Vương, Q.5", "x": -1.5, "y": -0.8, "gioMo": 480, "gioDong": 505, "khungGio": "08:00 - 08:25", "phutGiao": 10, "khoiLuong": 8, "bac": 0, "kmTuKhoDijkstra": 2.04},
      {"maDon": "DH02", "diaChi": "ĐH Khoa học Tự nhiên - Nguyễn Văn Cừ, Q.5", "x": -1.20, "y": -0.50, "gioMo": 495, "gioDong": 525, "khungGio": "08:15 - 08:45", "phutGiao": 10, "khoiLuong": 7, "bac": 1, "kmTuKhoDijkstra": 1.56},
      {"maDon": "DH03", "diaChi": "Chợ Bến Thành - Lê Lợi, Quận 1", "x": 2.50, "y": 0.40, "gioMo": 480, "gioDong": 505, "khungGio": "08:00 - 08:25", "phutGiao": 10, "khoiLuong": 9, "bac": 0, "kmTuKhoDijkstra": 3.04},
      {"maDon": "DH04", "diaChi": "Bưu điện Trung tâm TP - Công xã Paris, Q.1", "x": 3.00, "y": 1.80, "gioMo": 500, "gioDong": 530, "khungGio": "08:20 - 08:50", "phutGiao": 10, "khoiLuong": 6, "bac": 1, "kmTuKhoDijkstra": 4.20},
      {"maDon": "DH05", "diaChi": "ĐH Bách Khoa - 268 Lý Thường Kiệt, Q.10", "x": -4.20, "y": 2.00, "gioMo": 485, "gioDong": 510, "khungGio": "08:05 - 08:30", "phutGiao": 10, "khoiLuong": 13, "bac": 4, "kmTuKhoDijkstra": 5.58},
      {"maDon": "DH06", "diaChi": "ĐH Tôn Đức Thắng - Nguyễn Hữu Thọ, Q.7", "x": 1.80, "y": -6.20, "gioMo": 485, "gioDong": 510, "khungGio": "08:05 - 08:30", "phutGiao": 10, "khoiLuong": 9, "bac": 4, "kmTuKhoDijkstra": 7.75},
      {"maDon": "DH07", "diaChi": "Hồ Con Rùa - Võ Văn Tần, Quận 3", "x": 1.80, "y": 2.20, "gioMo": 505, "gioDong": 535, "khungGio": "08:25 - 08:55", "phutGiao": 10, "khoiLuong": 7, "bac": 0, "kmTuKhoDijkstra": 3.41},
      {"maDon": "DH08", "diaChi": "Sân bay Tân Sơn Nhất - Trường Sơn, Tân Bình", "x": -1.80, "y": 7.20, "gioMo": 485, "gioDong": 510, "khungGio": "08:05 - 08:30", "phutGiao": 12, "khoiLuong": 14, "bac": 3, "kmTuKhoDijkstra": 8.91},
      {"maDon": "DH09", "diaChi": "Landmark 81 - Điện Biên Phủ, Bình Thạnh", "x": 5.80, "y": 3.60, "gioMo": 490, "gioDong": 515, "khungGio": "08:10 - 08:35", "phutGiao": 10, "khoiLuong": 12, "bac": 3, "kmTuKhoDijkstra": 8.19},
      {"maDon": "DH10", "diaChi": "ĐH Ngân hàng - Hoàng Diệu 2, Thủ Đức", "x": 9.50, "y": 8.00, "gioMo": 500, "gioDong": 530, "khungGio": "08:20 - 08:50", "phutGiao": 10, "khoiLuong": 8, "bac": 4, "kmTuKhoDijkstra": 14.98}
    ],
    "canhXungDot": [
      {"u": 1, "v": 9, "lyDo": "Trùng khung giờ + cách 33 phút di chuyển"},
      {"u": 3, "v": 9, "lyDo": "Trùng khung giờ + cách 22 phút di chuyển"},
      {"u": 4, "v": 5, "lyDo": "Trùng khung giờ + cách 25 phút di chuyển"},
      {"u": 4, "v": 7, "lyDo": "Quá tải trọng xe (27kg > 25kg)"},
      {"u": 4, "v": 8, "lyDo": "Trùng khung giờ + cách 25 phút di chuyển"},
      {"u": 4, "v": 9, "lyDo": "Trùng khung giờ + cách 36 phút di chuyển"},
      {"u": 5, "v": 7, "lyDo": "Trùng khung giờ + cách 34 phút di chuyển"},
      {"u": 5, "v": 8, "lyDo": "Trùng khung giờ + cách 26 phút di chuyển"},
      {"u": 5, "v": 9, "lyDo": "Trùng khung giờ + cách 39 phút di chuyển"},
      {"u": 7, "v": 8, "lyDo": "Quá tải trọng xe (26kg > 25kg)"}
    ],
    "welshPowell": {
      "ten": "Welsh-Powell",
      "soMau": 5,
      "mauCuaDinh": [1, 0, 3, 1, 0, 1, 4, 2, 3, 2],
      "cacBuoc": [
        {"buoc": 1, "dinh": 4, "mau": 0, "bac": 4, "sat": 0, "giaiThich": "Xét DH05 (bậc deg = 4): không xung đột và đủ tải (13kg) → Gán vào Xe 1"},
        {"buoc": 2, "dinh": 1, "mau": 0, "bac": 1, "sat": 0, "giaiThich": "Xét DH02 (bậc deg = 1): không xung đột và đủ tải (20kg) → Gán vào Xe 1"},
        {"buoc": 3, "dinh": 5, "mau": 1, "bac": 4, "sat": 0, "giaiThich": "Xét DH06 (bậc deg = 4): không xung đột và đủ tải (9kg) → Gán vào Xe 2"},
        {"buoc": 4, "dinh": 3, "mau": 1, "bac": 1, "sat": 0, "giaiThich": "Xét DH04 (bậc deg = 1): không xung đột và đủ tải (15kg) → Gán vào Xe 2"},
        {"buoc": 5, "dinh": 0, "mau": 1, "bac": 0, "sat": 0, "giaiThich": "Xét DH01 (bậc deg = 0): không xung đột và đủ tải (23kg) → Gán vào Xe 2"},
        {"buoc": 6, "dinh": 9, "mau": 2, "bac": 4, "sat": 0, "giaiThich": "Xét DH10 (bậc deg = 4): không xung đột và đủ tải (8kg) → Gán vào Xe 3"},
        {"buoc": 7, "dinh": 7, "mau": 2, "bac": 3, "sat": 0, "giaiThich": "Xét DH08 (bậc deg = 3): không xung đột và đủ tải (22kg) → Gán vào Xe 3"},
        {"buoc": 8, "dinh": 8, "mau": 3, "bac": 3, "sat": 0, "giaiThich": "Xét DH09 (bậc deg = 3): không xung đột và đủ tải (12kg) → Gán vào Xe 4"},
        {"buoc": 9, "dinh": 2, "mau": 3, "bac": 0, "sat": 0, "giaiThich": "Xét DH03 (bậc deg = 0): không xung đột và đủ tải (21kg) → Gán vào Xe 4"},
        {"buoc": 10, "dinh": 6, "mau": 4, "bac": 0, "sat": 0, "giaiThich": "Xét DH07 (bậc deg = 0): không xung đột và đủ tải (7kg) → Gán vào Xe 5"}
      ],
      "loTrinhXe": [
        {"maXe": 1, "tongKg": 20, "tongKm": 11.83, "thuTuDon": [4, 1]},
        {"maXe": 2, "tongKg": 23, "tongKm": 23.54, "thuTuDon": [0, 5, 3]},
        {"maXe": 3, "tongKg": 22, "tongKm": 37.40, "thuTuDon": [7, 9]},
        {"maXe": 4, "tongKg": 21, "tongKm": 16.75, "thuTuDon": [2, 8]},
        {"maXe": 5, "tongKg": 7, "tongKm": 6.82, "thuTuDon": [6]}
      ]
    },
    "dsatur": {
      "ten": "DSATUR",
      "soMau": 5,
      "mauCuaDinh": [1, 0, 3, 1, 0, 1, 4, 2, 3, 2],
      "cacBuoc": [
        {"buoc": 1, "dinh": 4, "mau": 0, "bac": 4, "sat": 0, "giaiThich": "Chọn DH05 có độ bão hòa lớn nhất sat = 0 (deg = 4) → Gán vào Xe 1"},
        {"buoc": 2, "dinh": 5, "mau": 1, "bac": 4, "sat": 1, "giaiThich": "Chọn DH06 có độ bão hòa lớn nhất sat = 1 (deg = 4) → Gán vào Xe 2"},
        {"buoc": 3, "dinh": 9, "mau": 2, "bac": 4, "sat": 2, "giaiThich": "Chọn DH10 có độ bão hòa lớn nhất sat = 2 (deg = 4) → Gán vào Xe 3"},
        {"buoc": 4, "dinh": 7, "mau": 2, "bac": 3, "sat": 2, "giaiThich": "Chọn DH08 có độ bão hòa lớn nhất sat = 2 (deg = 3) → Gán vào Xe 3"},
        {"buoc": 5, "dinh": 8, "mau": 3, "bac": 3, "sat": 3, "giaiThich": "Chọn DH09 có độ bão hòa lớn nhất sat = 3 (deg = 3) → Gán vào Xe 4"},
        {"buoc": 6, "dinh": 1, "mau": 0, "bac": 1, "sat": 1, "giaiThich": "Chọn DH02 có độ bão hòa lớn nhất sat = 1 (deg = 1) → Gán vào Xe 1"},
        {"buoc": 7, "dinh": 3, "mau": 1, "bac": 1, "sat": 1, "giaiThich": "Chọn DH04 có độ bão hòa lớn nhất sat = 1 (deg = 1) → Gán vào Xe 2"},
        {"buoc": 8, "dinh": 0, "mau": 1, "bac": 0, "sat": 0, "giaiThich": "Chọn DH01 có độ bão hòa lớn nhất sat = 0 (deg = 0) → Gán vào Xe 2"},
        {"buoc": 9, "dinh": 2, "mau": 3, "bac": 0, "sat": 0, "giaiThich": "Chọn DH03 có độ bão hòa lớn nhất sat = 0 (deg = 0) → Gán vào Xe 4"},
        {"buoc": 10, "dinh": 6, "mau": 4, "bac": 0, "sat": 0, "giaiThich": "Chọn DH07 có độ bão hòa lớn nhất sat = 0 (deg = 0) → Gán vào Xe 5"}
      ],
      "loTrinhXe": [
        {"maXe": 1, "tongKg": 20, "tongKm": 11.83, "thuTuDon": [4, 1]},
        {"maXe": 2, "tongKg": 23, "tongKm": 23.54, "thuTuDon": [0, 5, 3]},
        {"maXe": 3, "tongKg": 22, "tongKm": 37.40, "thuTuDon": [7, 9]},
        {"maXe": 4, "tongKg": 21, "tongKm": 16.75, "thuTuDon": [2, 8]},
        {"maXe": 5, "tongKg": 7, "tongKm": 6.82, "thuTuDon": [6]}
      ]
    }
  }
];
