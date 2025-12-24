#!/bin/bash

# compile shaders
# slangc shader.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o slang.spv
# ./compile_shaders.sh

# compile project
cd build
cmake -G Ninja ..
ninja
