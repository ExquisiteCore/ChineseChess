@echo off
setlocal enabledelayedexpansion

:: ==============================
:: 🏗️ Qt 项目 Release 构建 & 打包脚本
:: 适用于 MinGW + Qt6 环境
:: ==============================

:: 设置控制台编码为 UTF-8，防止中文乱码
chcp 65001 >nul

:: Qt 安装路径
set QT_PATH=C:\Code\QT\6.10.0\mingw_64\bin

echo [1/8] 生成 Release 构建目录...
cmake -B build-release -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo ❌ CMake 配置失败！
    pause
    exit /b %errorlevel%
)

echo [2/8] 编译项目...
cmake --build build-release --config Release --target appChineseChess
if %errorlevel% neq 0 (
    echo ❌ 编译失败！
    pause
    exit /b %errorlevel%
)

echo [3/8] 创建发布目录...
if not exist release-package mkdir release-package

echo [4/8] 复制可执行文件...
copy /Y build-release\appChineseChess.exe release-package\

echo [5/8] 检查 windeployqt 工具...
where windeployqt >nul 2>nul
if %errorlevel% neq 0 (
    echo ⚠️ 未找到 windeployqt，使用指定路径...
    set WINDEPLOYQT=%QT_PATH%\windeployqt.exe
) else (
    for /f "delims=" %%i in ('where windeployqt') do set WINDEPLOYQT=%%i
)
echo 使用: %WINDEPLOYQT%

echo [6/8] 打包 Qt 依赖...
cd release-package
"%WINDEPLOYQT%" --qmldir ../qml appChineseChess.exe
cd ..

echo [7/8] 检查打包结果...
dir /b release-package\*.exe
dir /b release-package\*.dll | find /c /v "" > tmp_count.txt
set /p dllcount=<tmp_count.txt
del tmp_count.txt
echo 已打包 %dllcount% 个 DLL。

echo [8/8] 计算包体积...
for /f "tokens=3" %%a in ('dir /-c /s release-package ^| find "字节"') do set SIZE=%%a
echo 总大小: %SIZE% 字节

echo.
echo ✅ Release 版本已编译并打包成功！
echo 📁 位置: %cd%\release-package\
echo ▶️ 双击 appChineseChess.exe 即可运行游戏。
echo.

pause
