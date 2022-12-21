#!/bin/bash

rm -r shaders
rm Makefile
mkdir ../bin/
cmake -DCMAKE_BUILD_TYPE=Debug .. &&
make -j4 &&
./../bin/Croissant
