#!/bin/bash

# slangc shader.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o slang.spv
./programs/slang/bin/slangc shaders/shader.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o slang.spv


# glslc shaders/vertex_shader.vert -o shaders/vert.spv
# glslc shaders/fragment_shader.frag -o shaders/frag.spv

echo "finished shader compilation."
