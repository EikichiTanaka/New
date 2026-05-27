@echo off
taskkill /IM Project1.exe /F >nul 2>&1
echo [kill-game] Project1.exe terminated (if it was running).
