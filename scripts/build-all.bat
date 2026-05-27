@echo off
setlocal EnableExtensions
cd /d "%~dp0.."

call "%~dp0build.bat" debug
if errorlevel 1 exit /b 1

call "%~dp0build.bat" release
if errorlevel 1 exit /b 1

echo [build-all] Debug + Release x64 rebuild OK
exit /b 0
