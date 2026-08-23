#!/bin/bash

~/Downloads/1.4.357.1/x86_64/bin/slangc shaders/triangle.slang -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o shaders/slang.spv
