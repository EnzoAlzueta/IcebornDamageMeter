@echo off
setlocal
set "METER_VS="
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "METER_VS=%%i"
if not defined METER_VS (
  echo Install Visual Studio Build Tools with Desktop development with C++.
  exit /b 1
)
call "%METER_VS%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\build-core.ps1" %*
exit /b %errorlevel%
