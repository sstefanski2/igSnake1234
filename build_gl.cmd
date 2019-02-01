@echo off
setlocal
:: Set vars
call "%PROGRAMFILES(x86)%\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
set APPNAME=igSnake1234_gl
set COMPILER_OPT=/O1 /GS- /Oi-
:: Uncomment for debugging
rem set COMPILER_OPT=%COMPILER_OPT% /D_DEBUG
set LINKER_OPT=%APPNAME%.obj /SUBSYSTEM:WINDOWS user32.lib kernel32.lib gdi32.lib winmm.lib opengl32.lib
set CRINKLER_OPT=/ORDERTRIES:200 /HASHSIZE:2 /HASHTRIES:2 /COMPMODE:SLOW /UNSAFEIMPORT /TINYHEADER /TINYIMPORT

:start
if exist %APPNAME%.exe del %APPNAME%.exe

:: Compile and link
cl %APPNAME%.c /c %COMPILER_OPT%
if exist %APPNAME%.obj crinkler %LINKER_OPT% %CRINKLER_OPT% /OUT:%APPNAME%.exe 

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
