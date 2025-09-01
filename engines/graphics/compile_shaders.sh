#!/bin/bash

glslc shaders/vertex_shader.vert -o shaders/vert.spv
glslc shaders/fragment_shader.frag -o shaders/frag.spv

echo "finished shader compilation."
