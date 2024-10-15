#!/bin/env bash

CFLAGS='-std=c11 -O0 -g'

clang $CFLAGS -o proglosa code/main.c
