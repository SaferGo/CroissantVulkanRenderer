#!/bin/bash

rm -f -r shaders
rm -f Makefile
mkdir -f ../bin
cmake -DCMAKE_BUILD_TYPE=Release .. &&
make -j4 &&
./../bin/Croissant
