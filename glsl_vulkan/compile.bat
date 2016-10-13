@echo off
set COMPILER=%VK_SDK_PATH%\Bin32\glslangValidator.exe
for %%i in (*.vert,*.frag) do call :compile %%i
goto done

:compile
echo -------------
%COMPILER% -V %1 -o ../pack/assets/spv/%1.spv
echo ERRORLEVEL: %ERRORLEVEL%
if %ERRORLEVEL% equ 2 (
	pause
	exit
)
exit /b

:done
