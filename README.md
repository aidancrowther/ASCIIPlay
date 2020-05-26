# ASCIIPlay
A simple video player that live renders .mp4 containers to ASCII in console – written in GNU C

## Dependencies

primarily:
* ascii\_art.c (glhf with this one, we commented it out for ya)
* ffmpeg
* cmake
* make
* gcc

but apparently you'll need some indeterminate number of these too:
* build-essential
* pkg-config
* libgtk-3-dev
* libavcodec-dev
* libavformat-dev
* libswscale-dev
* libv4l-dev
* libxvidcore-dev
* libx264-dev
* libjpeg-dev
* libpng-dev
* libtiff-dev
* gfortran
* openexr
* libatlas-base-dev
* python3-dev
* python3-numpy
* libtbb2
* libtbb-dev
* libdc1394-22-dev

## Building

Currently the core visual library (the 'ascii_art.c' one) is for some reason unavailable so you won't see anything but it'll leak memory for you!
Use `$ cmake .` from the launchpoint with the dependencies installed and it should build a makefile.