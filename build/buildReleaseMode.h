#!/bin/bash

rm -r shaders
rm Makefile
mkdir ../bin/
cmake -DCMAKE_BUILD_TYPE=Release .. &&
make -j4 &&
./../bin/Croissant
