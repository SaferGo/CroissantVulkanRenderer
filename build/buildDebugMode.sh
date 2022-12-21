#!/bin/bash

rm -f -r shaders
rm -f Makefile
mkdir -f ../bin
cmake -DCMAKE_BUILD_TYPE=Debug .. &&
make -j4 &&
./../bin/Croissant
