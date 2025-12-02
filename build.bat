@echo off
setlocal enabledelayedexpansion

:: ==============================
:: 🛠️ Qt 项目 Debug 构建脚本
:: 适用于 MinGW + Qt6 环境
:: ==============================

:: 设置控制台编码为 UTF-8，防止中文乱码
chcp 65001 >nul

:: Qt 安装路径
set QT_DIR=C:\Code\QT\6.10.0\mingw_64
set MINGW_DIR=C:\Code\QT\Tools\mingw1310_64

:: 添加 MinGW 到 PATH
set PATH=%MINGW_DIR%\bin;%QT_DIR%\bin;%PATH%

echo [1/3] 生成 Debug 构建目录...
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=%QT_DIR%
if %errorlevel% neq 0 (
    echo ❌ CMake 配置失败！
    pause
    exit /b %errorlevel%
)

echo [2/3] 编译项目...
cmake --build build --config Debug
if %errorlevel% neq 0 (
    echo ❌ 编译失败！
    pause
    exit /b %errorlevel%
)

echo [3/3] 编译完成！
echo ✅ Debug 版本编译成功！
echo 📁 位置: %cd%\build\
echo.

pause
