#!/bin/bash
c++ emu.cc -std=c++11 -I ./hdr -o cpu && ./cpu boot;
