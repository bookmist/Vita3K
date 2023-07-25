@echo off
call "c:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
REM Generate project files for your last Visual Studio version you have
call cmake -S . -B build
pause
