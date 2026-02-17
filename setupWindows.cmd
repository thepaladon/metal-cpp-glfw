@echo off
setlocal EnableExtensions

set "repoRoot=%~dp0"
cd /d "%repoRoot%"

call :ensureCommand cmake Kitware.CMake "%ProgramFiles%\CMake\bin\cmake.exe" required
if errorlevel 1 exit /b 1

call :ensureCommand ninja Ninja-build.Ninja "%ProgramFiles%\Ninja\ninja.exe" required
if errorlevel 1 exit /b 1

call :initMsvcEnvironment
if errorlevel 1 exit /b 1

echo Configuring Ninja workspace...
cmake --preset ninja
if errorlevel 1 exit /b %errorlevel%
if exist "build\Garbage" attrib +h "build\Garbage"

echo Building Debug target...
cmake --build --preset ninja-debug
if errorlevel 1 exit /b %errorlevel%

echo Done. Executable: %repoRoot%build\Build\Debug\metal_cpp_test.exe
exit /b 0

:ensureCommand
set "commandName=%~1"
set "wingetId=%~2"
set "fallbackExe=%~3"
set "requireMode=%~4"

where "%commandName%" >nul 2>nul
if not errorlevel 1 goto :eof

where winget >nul 2>nul
if errorlevel 1 (
  echo ERROR: Missing '%commandName%' and winget is unavailable.
  echo Install '%wingetId%' manually, then re-run this script.
  exit /b 1
)

echo Installing %wingetId% via winget...
winget install --id "%wingetId%" --exact --silent --accept-package-agreements --accept-source-agreements
if errorlevel 1 (
  echo ERROR: winget failed while installing %wingetId%.
  exit /b 1
)

if exist "%ProgramFiles%\CMake\bin\cmake.exe" set "PATH=%ProgramFiles%\CMake\bin;%PATH%"
if exist "%ProgramFiles%\Ninja\ninja.exe" set "PATH=%ProgramFiles%\Ninja;%PATH%"
if exist "%ProgramFiles(x86)%\Ninja\ninja.exe" set "PATH=%ProgramFiles(x86)%\Ninja;%PATH%"
if exist "%LOCALAPPDATA%\Microsoft\WinGet\Links" set "PATH=%LOCALAPPDATA%\Microsoft\WinGet\Links;%PATH%"

where "%commandName%" >nul 2>nul
if not errorlevel 1 goto :eof

if exist "%fallbackExe%" (
  for %%I in ("%fallbackExe%") do set "PATH=%%~dpI;%PATH%"
  where "%commandName%" >nul 2>nul
  if not errorlevel 1 goto :eof
)

if /i "%requireMode%"=="required" (
  echo ERROR: '%commandName%' was installed but is still unavailable in PATH.
  echo Open a new terminal and run setupWindows.cmd again.
  exit /b 1
)

goto :eof

:initMsvcEnvironment
set "vswherePath=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswherePath%" (
  echo ERROR: Visual Studio detection tool not found.
  echo Install Visual Studio with Desktop development with C++.
  exit /b 1
)

set "vsInstallPath="
for /f "usebackq tokens=*" %%I in (`"%vswherePath%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "vsInstallPath=%%I"
if "%vsInstallPath%"=="" (
  echo ERROR: Visual Studio C++ workload not detected.
  echo Install Desktop development with C++ and re-run this script.
  exit /b 1
)

set "vsDevCmd=%vsInstallPath%\Common7\Tools\VsDevCmd.bat"
if not exist "%vsDevCmd%" (
  echo ERROR: VsDevCmd.bat not found at: %vsDevCmd%
  exit /b 1
)

call "%vsDevCmd%" -arch=x64 -host_arch=x64 >nul
if errorlevel 1 (
  echo ERROR: Failed to initialize Visual Studio developer environment.
  exit /b 1
)

goto :eof
