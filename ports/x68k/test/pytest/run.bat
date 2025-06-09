echo off
if not "%1" == "" goto start
echo run <type>...
echo type: basics extmod float import io micropython misc
goto exit

:start
set TYPE=%1

cd ..\..\..\..\tests\%TYPE%

set TESTS2X68K=..\..\ports\x68k
set MPY=%TESTS2X68K%\build\micropython.x
set LOG=%TESTS2X68K%\test\pytest\log-%TYPE%

del /y %LOG%
rmdir %LOG%
mkdir %LOG%
echo on
for %%a in (*.py) do %MPY% %%a > %LOG%\%%a
echo off

cd %TESTS2X68K%\test\pytest

shift
if not "%1" == "" goto start
:exit
