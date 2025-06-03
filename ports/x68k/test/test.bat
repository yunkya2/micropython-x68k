echo off
set MPY=..\build\micropython.x

screen
for %%a in (asmraster framebufgrp framebuftxt grplane iocscircle iocsline textline) do %MPY% ..\sample\%%a.py
screen
for %%a in (bench dosgetdpb) do %MPY% ..\sample\%%a.py
screen
for %%a in (sprite spviper spintr) do %MPY% ..\sample\%%a.py
screen
