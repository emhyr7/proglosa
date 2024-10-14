@echo off
setlocal

cd /D "%~dp0"

set CFLAGS=-std=c11 -O0 -g

clang %CFLAGS% -o proglosa.exe code\main.c
