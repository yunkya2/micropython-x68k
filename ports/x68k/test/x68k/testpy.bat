set MPY=..\..\build\micropython.x
set SAMPLE=..\..\sample

screen
for %%a in (asmraster framebufgrp framebuftxt grplane iocscircle iocsline textline) do %MPY% %SAMPLE%\%%a.py
screen
for %%a in (bench dosgetdpb) do %MPY% %SAMPLE%\%%a.py
screen
for %%a in (sprite spviper spintr) do %MPY% %SAMPLE%\%%a.py
screen
