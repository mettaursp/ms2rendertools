@echo off

setlocal EnableDelayedExpansion

set directory=%cd%
set checkDirectory=%cd%/HornetEngine

echo %directory%

:findpath
if exist "%checkDirectory%" (goto :pathfound)

    set directory2=%directory%/..
    set directory=%directory2%
    set checkDirectory=%directory2%/HornetEngine
    echo checking %checkDirectory%
    goto :findpath

:pathfound

echo found %checkDirectory%

%checkDirectory%/GameEngine/Release/GameEngine.exe %cd%/main.lua