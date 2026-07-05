#!/bin/bash

cmake -B build
cd build
cmake --build . --parallel
