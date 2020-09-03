#!/bin/bash

case "$(uname -s)" in
    Linux*) OPEN=xdg-open;;
    Darwin*) OPEN=open;;
    *) exit 1;;
esac

image=${1:-output.ppm}

make

echo "Generating ${image}"

./raytracer > ${image}

${OPEN} ${image} &
