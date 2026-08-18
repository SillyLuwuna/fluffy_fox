#!/bin/bash

type=$1

if [ -z "$type" ]
then
	type=Release
else
	type=$type
fi

cmake -B build -DCMAKE_BUILD_TYPE=$type # -DBUILD_TESTS=ON
cmake --build build --parallel
