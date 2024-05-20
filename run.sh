#!/bin/bash


source="source.txt"
IR="asm.txt"
MC="MachineCode.txt"



clang++ -std=c++17 Frontend.cpp -o Frontend
clang++ -std=c++17 Backend.cpp -o Backend

./Frontend $source $IR
./Backend $IR $MC


rm ./Backend
rm ./Frontend



