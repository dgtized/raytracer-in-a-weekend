#!/bin/bash

make clean
g++ -std=c++11 -pg src/main.cc -o raytracer
./render.sh

# Flat Graph
gprof raytracer gmon.out | head -20 > profiles/201111.random_scene.flat

# Using gprof2dot.py
# https://raw.githubusercontent.com/jrfonseca/gprof2dot/master/gprof2dot.py
# Call Graph Visualization
gprof --graph raytracer gmon.out | python gprof2dot.py |
    tee profiles/201111.random_scene.dot |
    dot -Tpng -o profiles/201111.random_scene.png
