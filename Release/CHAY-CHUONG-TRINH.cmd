@echo off
cd /d "%~dp0"
parcel.exe
if errorlevel 1 pause
