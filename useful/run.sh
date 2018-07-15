#!/usr/bin/env bash

WINE=/opt/wine-stable/bin/wine
WOW=/home/clanat/games/wow-vanilla/WoW.exe
LIB=/home/clanat/dev/neon-backend-cpp/cmake-build-debug/libneon_backend.so

wineserver -k
gdb -iex "set exec-wrapper env LD_PRELOAD=/home/clanat/dev/neon-backend-cpp/cmake-build-debug/libneon_backend.so" --args $WINE $WOW