@echo off
setlocal
cd /d "%~dp0"
title ShipperColor C++ - Do an Ly thuyet Do thi HCMUE

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
    echo [LOI] Khong tim thay trinh bien dich g++.
    pause
    exit /b 1
  )
)
echo Dang bien dich 3 file C++ (main.cpp, Graph.cpp, Algorithms.cpp)...
"%SHIPPER_COMPILER%" -std=c++17 -O2 -Wall -Wextra -static -o parcel.exe src/main.cpp src/Graph.cpp src/Algorithms.cpp
if errorlevel 1 (
  echo [LOI] Bien dich that bai.
  pause
  exit /b 1
)
echo Bien dich thanh cong: parcel.exe
if /I "%~1"=="--build" exit /b 0

:run
if /I "%~1"=="--test" (
  parcel.exe --test
  exit /b %errorlevel%
)
if /I "%~1"=="--menu" (
  parcel.exe --menu
  exit /b %errorlevel%
)
parcel.exe
if errorlevel 1 pause
