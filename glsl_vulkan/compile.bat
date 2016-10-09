set COMPILER=%VK_SDK_PATH%\Bin32\glslangValidator.exe

for %%i in (*.vert,*.frag) do %COMPILER% -V %%i -o ../pack/assets/spv/%%i.spv

pause
