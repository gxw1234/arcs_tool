@echo off
echo =================================================
echo           Qt Application Build Tool
echo =================================================

REM Set Qt environment paths
set QT_DIR=D:\qt\5.12.2\mingw73_32
set MINGW_DIR=D:\qt\Tools\mingw730_32\bin

echo Setting up environment...
set PATH=%QT_DIR%\bin;%MINGW_DIR%;%PATH%

echo.
echo Step 1: Running qmake...
qmake -o Makefile SimpleQtApp.pro

if not exist Makefile (
    echo ERROR: Failed to generate Makefile!
    goto ERROR
)

echo.
echo Step 2: Compiling application...
mingw32-make clean
mingw32-make -f Makefile.Release

if not exist bin\SimpleQtApp.exe (
    echo ERROR: Compilation failed!
    goto ERROR
)

echo.
echo Step 3: Deploying Qt dependencies...
windeployqt bin\SimpleQtApp.exe

echo.
echo =================================================
echo           Build and deployment completed!
echo =================================================
echo.
echo You can now run the application:
echo   bin\SimpleQtApp.exe
echo.

goto END

:ERROR
echo.
echo Build process failed!
echo.

:END
pause
