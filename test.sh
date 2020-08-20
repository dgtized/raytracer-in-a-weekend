#!/bin/bash

case "$(uname -s)" in
    Linux*) OPEN=xdg-open;;
    Darwin*) OPEN=open;;
    *) exit 1;;
esac

make

./raytracer > output.ppm

${OPEN} output.ppm &
