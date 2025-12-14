@echo on
title Build + Run Mimita (verbose)
setlocal EnableDelayedExpansion
cd /d "%~dp0"

echo.
echo === [STEP 1] Setting up paths ===

REM Check for environment variables first, then try common locations
if defined MIMITA_GPP (
    set "GPP=%MIMITA_GPP%"
) else if defined MINGW_ROOT (
    set "GPP=%MINGW_ROOT%\bin\g++.exe"
) else (
    REM Try common MinGW installation paths
    if exist "C:\mingw64\bin\g++.exe" (
        set "GPP=C:\mingw64\bin\g++.exe"
    ) else if exist "C:\msys64\mingw64\bin\g++.exe" (
        set "GPP=C:\msys64\mingw64\bin\g++.exe"
    ) else if exist "C:\Program Files\mingw-w64\*\mingw64\bin\g++.exe" (
        for %%d in ("C:\Program Files\mingw-w64\*\mingw64\bin\g++.exe") do set "GPP=%%d"
    ) else (
        echo ERROR: Could not find g++.exe
        echo Please set MIMITA_GPP environment variable or install MinGW-w64
        pause
        exit /b 1
    )
)

if defined MIMITA_GLFW_INC (
    set "GLFW_INC=%MIMITA_GLFW_INC%"
) else if defined GLFW_ROOT (
    set "GLFW_INC=%GLFW_ROOT%\include"
) else (
    REM Try common GLFW installation paths
    if exist "C:\glfw\include" (
        set "GLFW_INC=C:\glfw\include"
    ) else if exist "C:\Program Files\GLFW\include" (
        set "GLFW_INC=C:\Program Files\GLFW\include"
    ) else (
        echo WARNING: GLFW include directory not found. Trying pkg-config...
        set "GLFW_INC="
    )
)

if defined MIMITA_GLFW_LIB (
    set "GLFW_LIB=%MIMITA_GLFW_LIB%"
) else if defined GLFW_ROOT (
    set "GLFW_LIB=%GLFW_ROOT%\lib-mingw-w64"
) else (
    REM Try common GLFW library paths
    if exist "C:\glfw\lib-mingw-w64" (
        set "GLFW_LIB=C:\glfw\lib-mingw-w64"
    ) else if exist "C:\Program Files\GLFW\lib-mingw-w64" (
        set "GLFW_LIB=C:\Program Files\GLFW\lib-mingw-w64"
    ) else (
        echo WARNING: GLFW library directory not found. Trying pkg-config...
        set "GLFW_LIB="
    )
)

set "EXT_INC=external\include"
set "EXT_LIB=external\lib"

echo Using compiler: %GPP%
if defined GLFW_INC echo GLFW include:   %GLFW_INC%
if defined GLFW_LIB echo GLFW lib:       %GLFW_LIB%
echo External include: %EXT_INC%
echo External lib:     %EXT_LIB%
echo.

echo === [STEP 2] Gathering .cpp files ===
set SRC_FILES=
for /R src %%f in (*.cpp) do (
    echo Found: %%f
    set SRC_FILES=!SRC_FILES! "%%f"
)
echo.
echo Total source files collected:
echo !SRC_FILES!
echo.

echo === [STEP 3] Building mimita.exe ===
set "INCLUDE_FLAGS=-Iinclude -Isrc -I%EXT_INC%"
if defined GLFW_INC set "INCLUDE_FLAGS=%INCLUDE_FLAGS% -I%GLFW_INC%"

set "LIB_FLAGS=-L%EXT_LIB%"
if defined GLFW_LIB set "LIB_FLAGS=%LIB_FLAGS% -L%GLFW_LIB%"

"%GPP%" !SRC_FILES! src\glad.c ^
 -std=c++17 -O2 -Wall ^
 %INCLUDE_FLAGS% ^
 %LIB_FLAGS% ^
 -lglfw3dll -lopengl32 -lgdi32 -luser32 -ldwmapi ^
 -o mimita.exe -mconsole

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed with code %errorlevel%.
    pause
    exit /b
)

echo.
echo === [STEP 4] Build successful ===
echo.

echo === [STEP 5] Running mimita.exe ===
echo (Output will be written to run.log)
echo.

.\mimita.exe > run.log 2>&1
set "ERR=%ERRORLEVEL%"

echo.
echo === Program exited with code %ERR% ===
echo --- Last 40 lines of run.log ---
if exist "run.log" (
    powershell -Command "Get-Content -Path 'run.log' -Tail 40" 2>nul || type run.log | more
) else (
    echo No run.log file found.
)
echo.
pause
endlocal
