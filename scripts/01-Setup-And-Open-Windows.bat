@echo off
setlocal

set "scriptDir=%~dp0"
for %%I in ("%scriptDir%..") do set "repoRoot=%%~fI"

if not exist "%repoRoot%\scripts\bootstrapDeps.ps1" goto :missingHelpers
if not exist "%repoRoot%\scripts\generateProjectFiles.ps1" goto :missingHelpers

echo [1/2] Syncing dependencies...
powershell -NoProfile -ExecutionPolicy Bypass -File "%repoRoot%\scripts\bootstrapDeps.ps1"
if errorlevel 1 goto :failed

echo [2/2] Generating Visual Studio project files...
powershell -NoProfile -ExecutionPolicy Bypass -File "%repoRoot%\scripts\generateProjectFiles.ps1" -Action vs2022
if errorlevel 1 goto :failed

echo Opening solution...
start "" "%repoRoot%\build\ProjectFiles\vs2022\metalCppGlfw.sln"

echo.
echo Done.
if defined CI goto :noPause
pause
:noPause
exit /b 0

:failed
echo.
echo Failed. Review errors above.
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
