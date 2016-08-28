@echo off
setlocal
:: Set vars
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
set APPNAME=igSnake1234
set COMPILER_OPT=/O1 /GS- /Oi-
:: Uncomment for debugging
rem set COMPILER_OPT=%COMPILER_OPT% /D_DEBUG
set LINKER_OPT=%APPNAME%.obj /SUBSYSTEM:WINDOWS user32.lib kernel32.lib gdi32.lib winmm.lib
set CRINKLER_OPT=/ORDERTRIES:200 /HASHSIZE:2 /HASHTRIES:2 /COMPMODE:SLOW /UNSAFEIMPORT /TINYHEADER /TINYIMPORT

:start
if exist %APPNAME%.exe del %APPNAME%.exe
if exist %APPNAME%_mslinker.exe del %APPNAME%_mslinker.exe
if exist %APPNAME%_upx.exe del %APPNAME%_upx.exe

:: Compile
cl %APPNAME%.c /c %COMPILER_OPT%
if exist %APPNAME%.obj (
    :: MS linker
    link %LINKER_OPT% /RELEASE /NODEFAULTLIB /OUT:%APPNAME%_mslinker.exe 
    if exist upx.exe upx.exe %APPNAME%_mslinker.exe -o %APPNAME%_upx.exe
    :: Crinkler
    crinkler %LINKER_OPT% %CRINKLER_OPT% /OUT:%APPNAME%.exe 
)

:: Clean
del *.obj *.sys *.pdb

if exist %APPNAME%.exe (
    :: 'choice' is missing by default in WinXP and 2000
    choice /C:RC /n /M "Press C to recompile or R to run the program"
    if errorlevel 2 goto start
    if errorlevel 1 (
        call %APPNAME%.exe
        echo Press any key to recompile
        pause>nul
    )
) else (
    echo Press any key to recompile
    pause>nul
)
goto start

endlocal
