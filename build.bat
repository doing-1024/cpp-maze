@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem Build script for Windows (MinGW-w64 / MSYS2 / winlibs)
rem Usage:
rem   build.bat
rem   build.bat nostatic

pushd "%~dp0"

set "SRC=main.cpp"
set "OUT=main.exe"
set "INC=%CD%\include"
set "LIB=%CD%\lib"

if not exist "%SRC%" (
  echo [Error] "%SRC%" not found. Please run this script in the project root.
  popd
  exit /b 2
)

set "CXX=你的g++路径"
:cxx_found

if not defined CXX (
  echo [Error] g++ not found.
  echo Install a MinGW-w64 toolchain, then re-run.
  echo For MSYS2: install mingw-w64-x86_64-gcc and use the MinGW64 shell.
  popd
  exit /b 2
)

set "STATIC_FLAG=-static"
if /i "%~1"=="nostatic" set "STATIC_FLAG="

echo 编译中
"%CXX%" "%SRC%" -o "%OUT%" ^
  -I"%INC%" ^
  -L"%LIB%" ^
  -lgraphics ^
  -lgdiplus -luuid -lmsimg32 -lgdi32 -limm32 -lole32 -loleaut32 -lwinmm ^
  -mwindows ^
  %STATIC_FLAG%

if errorlevel 1 (
  echo [Error] build failed. Scroll up for the real linker error ^(undefined reference / cannot find -lXXX^).
  popd
  exit /b 1
)

echo 编译完成
popd
exit /b 0

