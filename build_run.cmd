@echo off
setlocal EnableExtensions

set "MODE=%~1"
if "%MODE%"=="" (
  echo Usage: %~nx0 ^<r^|d^>
  echo   r = Release
  echo   d = Debug
  exit /b 1
)

if /I "%MODE%"=="r" (
  set "CFG=Release"
) else if /I "%MODE%"=="d" (
  set "CFG=Debug"
) else (
  echo Invalid option: %MODE%
  echo Usage: %~nx0 ^<r^|d^>
  exit /b 1
)

set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
set "BUILD_DIR=%ROOT%\build"
set "APP=QtSnapLite.exe"

echo [1/3] Configuring CMake...
cmake -S "%ROOT%" -B "%BUILD_DIR%"
if errorlevel 1 exit /b 1

echo [2/3] Building %CFG%...
cmake --build "%BUILD_DIR%" --config %CFG% --target QtSnapLite
if errorlevel 1 exit /b 1

set "EXE_PATH=%BUILD_DIR%\%CFG%\%APP%"
if not exist "%EXE_PATH%" set "EXE_PATH=%BUILD_DIR%\%APP%"

if not exist "%EXE_PATH%" (
  echo Build succeeded but executable not found.
  echo Tried:
  echo   "%BUILD_DIR%\%CFG%\%APP%"
  echo   "%BUILD_DIR%\%APP%"
  exit /b 1
)

echo [3/3] Running %EXE_PATH%
start "" "%EXE_PATH%"
exit /b 0
