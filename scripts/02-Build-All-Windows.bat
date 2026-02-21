@echo off
setlocal

set "scriptDir=%~dp0"
for %%I in ("%scriptDir%..") do set "repoRoot=%%~fI"
set "slnPath=%repoRoot%\build\ProjectFiles\vs2022\metalCppGlfw.sln"
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "msbuildPath="

if not exist "%repoRoot%\scripts\bootstrapDeps.ps1" goto :missingHelpers
if not exist "%repoRoot%\scripts\generateProjectFiles.ps1" goto :missingHelpers

echo [1/4] Syncing dependencies...
powershell -NoProfile -ExecutionPolicy Bypass -File "%repoRoot%\scripts\bootstrapDeps.ps1"
if errorlevel 1 goto :failed

echo [2/4] Generating Visual Studio project files...
powershell -NoProfile -ExecutionPolicy Bypass -File "%repoRoot%\scripts\generateProjectFiles.ps1" -Action vs2022
if errorlevel 1 goto :failed

echo [3/4] Locating MSBuild...
if exist "%vswhere%" (
    for /f "usebackq delims=" %%I in (`"%vswhere%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
        set "msbuildPath=%%I"
    )
)
if not defined msbuildPath (
    echo Could not find MSBuild automatically.
    echo Open and build this solution manually:
    echo %slnPath%
    goto :failed
)

echo [4/4] Building Debug and Release...
"%msbuildPath%" "%slnPath%" /m /p:Configuration=Debug /p:Platform=x64
if errorlevel 1 goto :failed
"%msbuildPath%" "%slnPath%" /m /p:Configuration=Release /p:Platform=x64
if errorlevel 1 goto :failed

echo.
echo Build succeeded.
echo Debug executable:   %repoRoot%\build\Build\vs2022\Debug\metalCppTest.exe
echo Release executable: %repoRoot%\build\Build\vs2022\Release\metalCppTest.exe
if defined CI goto :noPause
pause
:noPause
exit /b 0

:failed
echo.
echo Build failed. Review errors above.
if defined CI goto :noPauseFailed
pause
:noPauseFailed
exit /b 1

:missingHelpers
echo.
echo Missing helper scripts under "%repoRoot%\scripts".
echo Expected:
echo   bootstrapDeps.ps1
echo   generateProjectFiles.ps1
if defined CI goto :noPauseMissing
pause
:noPauseMissing
exit /b 1
