@echo off
setlocal EnableExtensions
cd /d "%~dp0.."

echo [build] Working directory: %CD%

REM Kill running game if it locks the linker output
taskkill /IM Project1.exe /F >nul 2>&1

set "MSBUILD="
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
  set "MSBUILD=%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
  set "MSBUILD=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
  set "MSBUILD=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
)
if not defined MSBUILD if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
  set "MSBUILD=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
)

if not defined MSBUILD (
  echo [build] ERROR: MSBuild.exe not found. Install Visual Studio with C++ workload.
  exit /b 1
)

set "CONFIG=Debug"
set "PLATFORM=x64"
set "TARGET=Rebuild"
if /I "%~1"=="release" set "CONFIG=Release"
if /I "%~1"=="debug" set "CONFIG=Debug"
if /I "%~2"=="build" set "TARGET=Build"

echo [build] Using: %MSBUILD%
echo [build] Target: %TARGET% %CONFIG% %PLATFORM%
"%MSBUILD%" Project1.vcxproj /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /t:%TARGET% /m /v:minimal
set "ERR=%ERRORLEVEL%"
if %ERR% neq 0 (
  echo [build] FAILED with exit code %ERR%
  exit /b %ERR%
)

echo [build] OK - output: x64\%CONFIG%\Project1.exe
exit /b 0
