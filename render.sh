#!/bin/bash

set -e

case "$(uname -s)" in
    Linux*) OPEN=xdg-open;;
    Darwin*) OPEN=open;;
    *) exit 1;;
esac

make

if [[ $1 == mpv ]]; then
    echo "Generating video"
    ./raytracer > foo.video
    mpv --no-correct-pts --fps=30 --keep-open=yes foo.video
else
    image=${1:-output.ppm}
    echo "Generating ${image}"
    ./raytracer > ${image}
    ${OPEN} ${image} &
fi
