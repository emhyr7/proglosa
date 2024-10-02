@echo off
setlocal

cd /D "%~dp0"

set CFLAGS=-std=c11 -O0 -g

set FILES=proglosa/proglosa.c proglosa/proglosa_parsing.c

clang %CFLAGS% -o proglosa.exe %FILES%
