#!/bin/sh

echo "LibGGI speed test"

echo
echo "memory target at 8bpp:"
GGI_DISPLAY="memory" GGI_DEFMODE="640x480 [P8/8]" ./speed --all

echo
echo "memory target at 16bpp:"
GGI_DISPLAY="memory" GGI_DEFMODE="640x480 [C16/16]" ./speed --all

echo
echo "memory target at 24bpp:"
GGI_DISPLAY="memory" GGI_DEFMODE="640x480 [C24/32]" ./speed --all
