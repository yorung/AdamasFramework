adb logcat -c
adb shell am start -a android.intent.action.MAIN -n com.pinotnoir.adamas/common.pinotnoir.glactivity.PinotGLActivity

for /f "skip=1 delims=" %%x in ('wmic os get localdatetime') do if not defined X set X=%%x

if not exist $logs mkdir $logs
adb logcat > $logs\$logcat_%X:~0,14%.log
pause
