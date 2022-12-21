#!/bin/bash

rm -f -r shaders
rm -f Makefile

if [ !-d "../bin" ]; then
   mkdir ../bin
fi

cmake -DCMAKE_BUILD_TYPE=Release .. &&
make -j4 &&
./../bin/Croissant
