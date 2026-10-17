#!/bin/bash

type=$1

# if [ -z "$type" ]
# then
# 	type=Release
# else
# 	type=$type
# fi
if [ -z "$type" ]
then
	type=Debug
else
	type=$type
fi

cmake -B build -D CMAKE_BUILD_TYPE=$type # -DBUILD_TESTS=ON
cmake --build build --parallel
