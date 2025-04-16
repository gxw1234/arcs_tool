@echo off
echo =================================================
echo           Qt应用程序编译与部署工具
echo =================================================

REM 设置Qt环境变量（基于之前找到的Qt路径）
set QT_DIR=D:\qt\5.12.2\mingw73_32
set MINGW_DIR=D:\qt\Tools\mingw730_32\bin

echo 正在设置环境变量...
set PATH=%QT_DIR%\bin;%MINGW_DIR%;%PATH%

echo.
echo 步骤1: 运行qmake生成Makefile...
qmake -o Makefile SimpleQtApp.pro

if not exist Makefile (
    echo 错误：无法生成Makefile！
    goto ERROR
)

echo.
echo 步骤2: 编译应用程序...
mingw32-make clean
mingw32-make -f Makefile.Release

if not exist release\SimpleQtApp.exe (
    echo 错误：编译失败！
    goto ERROR
)

echo.
echo 步骤3: 部署Qt依赖项...
windeployqt release\SimpleQtApp.exe

echo.
echo =================================================
echo           编译和部署成功完成！
echo =================================================
echo.
echo 您可以通过执行以下命令运行应用程序：
echo   release\SimpleQtApp.exe
echo.
echo 是否立即运行应用程序？(Y/N)
set /p RUN_APP=
if /i "%RUN_APP%"=="Y" (
    start release\SimpleQtApp.exe
)

goto END

:ERROR
echo.
echo 构建过程失败！
echo.

:END
pause
