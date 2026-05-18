@echo off
cd /d "%~dp0"
set PATH=%CD%\DLLs;%PATH%
main.exe
pause
