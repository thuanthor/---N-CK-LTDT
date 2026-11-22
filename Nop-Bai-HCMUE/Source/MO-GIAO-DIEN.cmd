@echo off
setlocal
cd /d "%~dp0"
title ShipperGraph - Do an Ly thuyet Do thi (C++17 + HTML/CSS/JS)

if /I "%~1"=="--build" goto build
if not exist parcel.exe goto build
goto run

:build
set "SHIPPER_COMPILER=g++"
where g++ >nul 2>nul
if errorlevel 1 (
  if exist "C:\msys64\ucrt64\bin\g++.exe" (
    set "SHIPPER_COMPILER=C:\msys64\ucrt64\bin\g++.exe"
  ) else (
    echo [LOI] Khong tim thay trinh bien dich g++. Hay cai MinGW-w64 hoac MSYS2 UCRT64.
    pause
    exit /b 1
  )
)
echo Dang bien dich ma nguon C++17 tu thu muc src/...
"%SHIPPER_COMPILER%" -std=c++17 -O2 -Wall -Wextra -static -o parcel-moi.exe src/main.cpp -lws2_32
if errorlevel 1 (
  echo [LOI] Bien dich that bai. Xem thong bao loi phia tren.
  pause
  exit /b 1
)
move /y parcel-moi.exe parcel.exe >nul
if errorlevel 1 (
  echo [LOI] Khong ghi de duoc parcel.exe. Hay dong cua so chuong trinh cu bang Ctrl+C roi thu lai.
  pause
  exit /b 1
)
echo Bien dich thanh cong: parcel.exe
if /I "%~1"=="--build" exit /b 0

:run
if /I "%~1"=="--test" (
  parcel.exe --self-test
  exit /b %errorlevel%
)
echo Dang khoi dong may chu C++17 va mo giao dien Web...
echo Giu cua so nay mo khi su dung. Nhan Ctrl+C de dung chuong trinh.
parcel.exe
if errorlevel 1 pause
