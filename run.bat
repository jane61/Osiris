@echo OFF 
echo *ONLY WORKS FOR VISUAL STUDIO 2017* 
set /p Product=Enter your Visual Studio Product (Community, Enterprise, Professional): 
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\%Product%\VC\Auxiliary\Build\vcvarsall.bat" x86
echo Starting Build for all Projects with proposed changes


set /p Solution=Enter your solution file (Name + Extension): 
devenv "%~dp0%Solution% " /build Release 
echo 

echo All builds completed. 
pause
