set MPY=..\..\build\micropython.x
set MPC=..\..\mpyconv\mpyconv.x
set SAMPLE=..\..\sample

del /y *.mpy
for %%a in (asmraster framebufgrp framebuftxt grplane iocscircle iocsline textline) do %MPC% %SAMPLE%\%%a.py
for %%a in (bench dosgetdpb) do %MPC% %SAMPLE%\%%a.py
for %%a in (sprite spviper spintr) do %MPC% %SAMPLE%\%%a.py
move %SAMPLE%\*.mpy .
for %%a in (asmraster framebuf framebuftxt grplane iocscircle iocsline textline) do %MPY% -m %%a
for %%a in (bench dosgetdpb) do %MPY% -m %%a
for %%a in (sprite spviper spintr) do %MPY% -m %%a
del /y *.mpy
