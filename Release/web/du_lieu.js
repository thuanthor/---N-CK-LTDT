// File sinh tu dong tu chuong trinh C++ (src/main.cpp)
window.DU_LIEU_TU_CPP = [
  {
    "tenBo": "Bộ 1: 10 đơn giao hàng giờ cao điểm sáng TP.HCM",
    "taiToiDa": 25,
    "laDonDoThiVoHuong": true,
    "soCanh": 10,
    "soMienLienThong": 4,
    "soMau": 5,
    "donHang": [
      {"maDon": "DH01", "diaChi": "ĐH Sư Phạm CS1 - 280 An Dương Vương, Q.5", "x": -1.5, "y": -0.8, "gioMo": 480, "gioDong": 505, "khungGio": "08:00 - 08:25", "phutGiao": 10, "khoiLuong": 8, "bac": 0, "coLap": true, "dinhTreo": false, "mienLienThong": 1, "kcTuKho": 2.04},
      {"maDon": "DH02", "diaChi": "ĐH Khoa học Tự nhiên - Nguyễn Văn Cừ, Q.5", "x": -1.2, "y": -0.5, "gioMo": 495, "gioDong": 525, "khungGio": "08:15 - 08:45", "phutGiao": 10, "khoiLuong": 7, "bac": 1, "coLap": false, "dinhTreo": true, "mienLienThong": 2, "kcTuKho": 1.56},
      {"maDon": "DH03", "diaChi": "Chợ Bến Thành - Lê Lợi, Quận 1", "x": 2.5, "y": 0.4, "gioMo": 480, "gioDong": 505, "khungGio": "08:00 - 08:25", "phutGiao": 10, "khoiLuong": 9, "bac": 0, "coLap": true, "dinhTreo": false, "mienLienThong": 3, "kcTuKho": 3.04},
      {"maDon": "DH04", "diaChi": "Bưu điện Trung tâm TP - Công xã Paris, Q.1", "x": 3, "y": 1.8, "gioMo": 500, "gioDong": 530, "khungGio": "08:20 - 08:50", "phutGiao": 10, "khoiLuong": 6, "bac": 1, "coLap": false, "dinhTreo": true, "mienLienThong": 2, "kcTuKho": 4.20},
      {"maDon": "DH05", "diaChi": "ĐH Bách Khoa - 268 Lý Thường Kiệt, Q.10", "x": -4.2, "y": 2, "gioMo": 485, "gioDong": 510, "khungGio": "08:05 - 08:30", "phutGiao": 10, "khoiLuong": 13, "bac": 4, "coLap": false, "dinhTreo": false, "mienLienThong": 2, "kcTuKho": 5.58},
      {"maDon": "DH06", "diaChi": "ĐH Tôn Đức Thắng - Nguyễn Hữu Thọ, Q.7", "x": 1.8, "y": -6.2, "gioMo": 485, "gioDong": 510, "khungGio": "08:05 - 08:30", "phutGiao": 10, "khoiLuong": 9, "bac": 4, "coLap": false, "dinhTreo": false, "mienLienThong": 2, "kcTuKho": 7.75},
      {"maDon": "DH07", "diaChi": "Hồ Con Rùa - Võ Văn Tần, Quận 3", "x": 1.8, "y": 2.2, "gioMo": 505, "gioDong": 535, "khungGio": "08:25 - 08:55", "phutGiao": 10, "khoiLuong": 7, "bac": 0, "coLap": true, "dinhTreo": false, "mienLienThong": 4, "kcTuKho": 3.41},
      {"maDon": "DH08", "diaChi": "Sân bay Tân Sơn Nhất - Trường Sơn, Tân Bình", "x": -1.8, "y": 7.2, "gioMo": 485, "gioDong": 510, "khungGio": "08:05 - 08:30", "phutGiao": 12, "khoiLuong": 14, "bac": 3, "coLap": false, "dinhTreo": false, "mienLienThong": 2, "kcTuKho": 8.91},
      {"maDon": "DH09", "diaChi": "Landmark 81 - Điện Biên Phủ, Bình Thạnh", "x": 5.8, "y": 3.6, "gioMo": 490, "gioDong": 515, "khungGio": "08:10 - 08:35", "phutGiao": 10, "khoiLuong": 12, "bac": 3, "coLap": false, "dinhTreo": false, "mienLienThong": 2, "kcTuKho": 8.19},
      {"maDon": "DH10", "diaChi": "ĐH Ngân hàng - Hoàng Diệu 2, Thủ Đức", "x": 9.5, "y": 8, "gioMo": 500, "gioDong": 530, "khungGio": "08:20 - 08:50", "phutGiao": 10, "khoiLuong": 8, "bac": 4, "coLap": false, "dinhTreo": false, "mienLienThong": 2, "kcTuKho": 14.90}
    ],
    "canhXungDot": [
      {"u": 1, "v": 9, "lyDo": "Trùng khung giờ + cách 33 phút chạy xe"},
      {"u": 3, "v": 9, "lyDo": "Trùng khung giờ + cách 22 phút chạy xe"},
      {"u": 4, "v": 5, "lyDo": "Trùng khung giờ + cách 25 phút chạy xe"},
      {"u": 4, "v": 7, "lyDo": "Quá tải trọng xe (27kg > 25kg)"},
      {"u": 4, "v": 8, "lyDo": "Trùng khung giờ + cách 25 phút chạy xe"},
      {"u": 4, "v": 9, "lyDo": "Trùng khung giờ + cách 36 phút chạy xe"},
      {"u": 5, "v": 7, "lyDo": "Trùng khung giờ + cách 34 phút chạy xe"},
      {"u": 5, "v": 8, "lyDo": "Trùng khung giờ + cách 26 phút chạy xe"},
      {"u": 5, "v": 9, "lyDo": "Trùng khung giờ + cách 39 phút chạy xe"},
      {"u": 7, "v": 8, "lyDo": "Quá tải trọng xe (26kg > 25kg)"}
    ],
    "mauCuaDinh": [3, 0, 4, 1, 0, 1, 1, 2, 3, 2],
    "cacBuoc": [
      {"buoc": 1, "dinh": 4, "mau": 0, "bac": 4, "giaiThich": "Xét DH05 (bậc deg = 4): không kề đỉnh cùng màu, tổng tải 13kg → Tô Màu 1 (Xe 1)"},
      {"buoc": 2, "dinh": 1, "mau": 0, "bac": 1, "giaiThich": "Xét DH02 (bậc deg = 1): không kề đỉnh cùng màu, tổng tải 20kg → Tô Màu 1 (Xe 1)"},
      {"buoc": 3, "dinh": 5, "mau": 1, "bac": 4, "giaiThich": "Xét DH06 (bậc deg = 4): không kề đỉnh cùng màu, tổng tải 9kg → Tô Màu 2 (Xe 2)"},
      {"buoc": 4, "dinh": 3, "mau": 1, "bac": 1, "giaiThich": "Xét DH04 (bậc deg = 1): không kề đỉnh cùng màu, tổng tải 15kg → Tô Màu 2 (Xe 2)"},
      {"buoc": 5, "dinh": 6, "mau": 1, "bac": 0, "giaiThich": "Xét DH07 (bậc deg = 0): không kề đỉnh cùng màu, tổng tải 22kg → Tô Màu 2 (Xe 2)"},
      {"buoc": 6, "dinh": 9, "mau": 2, "bac": 4, "giaiThich": "Xét DH10 (bậc deg = 4): không kề đỉnh cùng màu, tổng tải 8kg → Tô Màu 3 (Xe 3)"},
      {"buoc": 7, "dinh": 7, "mau": 2, "bac": 3, "giaiThich": "Xét DH08 (bậc deg = 3): không kề đỉnh cùng màu, tổng tải 22kg → Tô Màu 3 (Xe 3)"},
      {"buoc": 8, "dinh": 8, "mau": 3, "bac": 3, "giaiThich": "Xét DH09 (bậc deg = 3): không kề đỉnh cùng màu, tổng tải 12kg → Tô Màu 4 (Xe 4)"},
      {"buoc": 9, "dinh": 0, "mau": 3, "bac": 0, "giaiThich": "Xét DH01 (bậc deg = 0): không kề đỉnh cùng màu, tổng tải 20kg → Tô Màu 4 (Xe 4)"},
      {"buoc": 10, "dinh": 2, "mau": 4, "bac": 0, "giaiThich": "Xét DH03 (bậc deg = 0): không kề đỉnh cùng màu, tổng tải 9kg → Tô Màu 5 (Xe 5)"}
    ],
    "loTrinhXe": [
      {"maXe": 1, "tongKg": 20, "tongKm": 11.83, "dsDon": [4, 1]},
      {"maXe": 2, "tongKg": 22, "tongKm": 22.38, "dsDon": [5, 3, 6]},
      {"maXe": 3, "tongKg": 22, "tongKm": 37.40, "dsDon": [7, 9]},
      {"maXe": 4, "tongKg": 20, "tongKm": 20.46, "dsDon": [0, 8]},
      {"maXe": 5, "tongKg": 9, "tongKm": 6.08, "dsDon": [2]}
    ]
  },
  {
    "tenBo": "Bộ 2: 8 đơn hàng (minh họa Đỉnh cô lập, Đỉnh treo & Liên thông)",
    "taiToiDa": 25,
    "laDonDoThiVoHuong": true,
    "soCanh": 4,
    "soMienLienThong": 4,
    "soMau": 3,
    "donHang": [
      {"maDon": "DH01", "diaChi": "280 An Dương Vương, Phường 4, Quận 5", "x": -1.5, "y": -0.8, "gioMo": 480, "gioDong": 500, "khungGio": "08:00 - 08:20", "phutGiao": 10, "khoiLuong": 8, "bac": 1, "coLap": false, "dinhTreo": true, "mienLienThong": 1, "kcTuKho": 2.04},
      {"maDon": "DH02", "diaChi": "45 Lê Lợi, Bến Nghé, Quận 1", "x": 2.5, "y": 0.4, "gioMo": 480, "gioDong": 500, "khungGio": "08:00 - 08:20", "phutGiao": 10, "khoiLuong": 9, "bac": 2, "coLap": false, "dinhTreo": false, "mienLienThong": 1, "kcTuKho": 3.04},
      {"maDon": "DH03", "diaChi": "268 Lý Thường Kiệt, Phường 14, Quận 10", "x": -4.2, "y": 2, "gioMo": 485, "gioDong": 505, "khungGio": "08:05 - 08:25", "phutGiao": 10, "khoiLuong": 10, "bac": 1, "coLap": false, "dinhTreo": true, "mienLienThong": 1, "kcTuKho": 5.58},
      {"maDon": "DH04", "diaChi": "59C Nguyễn Đình Chiểu, Quận 3", "x": 1.2, "y": 1.8, "gioMo": 495, "gioDong": 520, "khungGio": "08:15 - 08:40", "phutGiao": 10, "khoiLuong": 7, "bac": 0, "coLap": true, "dinhTreo": false, "mienLienThong": 2, "kcTuKho": 2.60},
      {"maDon": "DH05", "diaChi": "19 Nguyễn Hữu Thọ, Tân Phong, Quận 7", "x": 1.8, "y": -6.5, "gioMo": 570, "gioDong": 590, "khungGio": "09:30 - 09:50", "phutGiao": 10, "khoiLuong": 8, "bac": 1, "coLap": false, "dinhTreo": true, "mienLienThong": 3, "kcTuKho": 8.09},
      {"maDon": "DH06", "diaChi": "702 Nguyễn Văn Linh, Quận 7", "x": 4.5, "y": -7, "gioMo": 570, "gioDong": 590, "khungGio": "09:30 - 09:50", "phutGiao": 10, "khoiLuong": 9, "bac": 1, "coLap": false, "dinhTreo": true, "mienLienThong": 3, "kcTuKho": 9.99},
      {"maDon": "DH07", "diaChi": "01 Võ Văn Ngân, Linh Chiểu, Thủ Đức", "x": 9, "y": 7.5, "gioMo": 575, "gioDong": 595, "khungGio": "09:35 - 09:55", "phutGiao": 10, "khoiLuong": 6, "bac": 2, "coLap": false, "dinhTreo": false, "mienLienThong": 3, "kcTuKho": 14.06},
      {"maDon": "DH08", "diaChi": "273 An Dương Vương, Phường 3, Quận 5", "x": -1, "y": -0.9, "gioMo": 660, "gioDong": 710, "khungGio": "11:00 - 11:50", "phutGiao": 10, "khoiLuong": 5, "bac": 0, "coLap": true, "dinhTreo": false, "mienLienThong": 4, "kcTuKho": 1.61}
    ],
    "canhXungDot": [
      {"u": 0, "v": 1, "lyDo": "Trùng khung giờ + cách 11 phút chạy xe"},
      {"u": 1, "v": 2, "lyDo": "Trùng khung giờ + cách 17 phút chạy xe"},
      {"u": 4, "v": 6, "lyDo": "Trùng khung giờ + cách 38 phút chạy xe"},
      {"u": 5, "v": 6, "lyDo": "Trùng khung giờ + cách 37 phút chạy xe"}
    ],
    "mauCuaDinh": [2, 0, 1, 0, 1, 2, 0, 1],
    "cacBuoc": [
      {"buoc": 1, "dinh": 1, "mau": 0, "bac": 2, "giaiThich": "Xét DH02 (bậc deg = 2): không kề đỉnh cùng màu, tổng tải 9kg → Tô Màu 1 (Xe 1)"},
      {"buoc": 2, "dinh": 6, "mau": 0, "bac": 2, "giaiThich": "Xét DH07 (bậc deg = 2): không kề đỉnh cùng màu, tổng tải 15kg → Tô Màu 1 (Xe 1)"},
      {"buoc": 3, "dinh": 3, "mau": 0, "bac": 0, "giaiThich": "Xét DH04 (bậc deg = 0): không kề đỉnh cùng màu, tổng tải 22kg → Tô Màu 1 (Xe 1)"},
      {"buoc": 4, "dinh": 2, "mau": 1, "bac": 1, "giaiThich": "Xét DH03 (bậc deg = 1): không kề đỉnh cùng màu, tổng tải 10kg → Tô Màu 2 (Xe 2)"},
      {"buoc": 5, "dinh": 4, "mau": 1, "bac": 1, "giaiThich": "Xét DH05 (bậc deg = 1): không kề đỉnh cùng màu, tổng tải 18kg → Tô Màu 2 (Xe 2)"},
      {"buoc": 6, "dinh": 7, "mau": 1, "bac": 0, "giaiThich": "Xét DH08 (bậc deg = 0): không kề đỉnh cùng màu, tổng tải 23kg → Tô Màu 2 (Xe 2)"},
      {"buoc": 7, "dinh": 5, "mau": 2, "bac": 1, "giaiThich": "Xét DH06 (bậc deg = 1): không kề đỉnh cùng màu, tổng tải 9kg → Tô Màu 3 (Xe 3)"},
      {"buoc": 8, "dinh": 0, "mau": 2, "bac": 1, "giaiThich": "Xét DH01 (bậc deg = 1): không kề đỉnh cùng màu, tổng tải 17kg → Tô Màu 3 (Xe 3)"}
    ],
    "loTrinhXe": [
      {"maXe": 1, "tongKg": 22, "tongKm": 30.98, "dsDon": [1, 3, 6]},
      {"maXe": 2, "tongKg": 23, "tongKm": 27.20, "dsDon": [2, 4, 7]},
      {"maXe": 3, "tongKg": 17, "tongKm": 22.38, "dsDon": [0, 5]}
    ]
  }
];
