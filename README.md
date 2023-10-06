# ASCIIPlay

ASCIIPlay is a video player designed for the purpouse of rendering to ASCII any video file given to it, inspired by towel.blinkenlights.nl. The reasoning for my making my  own ASCII video player is due to the lack of an existing ASCII renderer with support for subtitle or audio playback. Thus I hope for  ASCIIPlay to develop to fill this niche.

A server is currently publicly accessible to view an example of this project, and can be accessed using: `ssh anonymous@telnetflix.com -p 20`
  
![](ASCIIPlay.gif)
  
## Goals

My main goal for ASCIIPlay is to develop a fully functional video player with a variety of features, while also practicing my C development and trying features I haven't previously used much. To this regard, I have definitely learned much more about data manipulation and threading in C programming, and I hope to expand into more practice as the project moves forward.
  
## Status
  
ASCIIPlay is currently stable, with many features being planned and added frequently.

Currently Supports:

- Reading in a variety of video formats such ash mp4, mkv, ...
- Subtitle rendering using SRT formatted subtitles
- Frame limiting for lower end systems or situations where lower bitrate is necessary
- Black and White ASCII rendering only at this time
   
## Roadmap

- [x] Load and read video files
   - [x] Render ASCII representation
   - [ ] Use custom ASCII renderer to remove closed source dependencies
   - [ ] Colour support
   - [ ] Audio renderer
- [x] Add subtitle support
   - [ ] More subtitle filetypes
   - [ ] Alternate subtitle rendering modes
- [x] Performance improvements
   - [x] framerate limiting
   - [x] Implement frame buffer
   - [x] Make use of threading
- [x] Support Resizing
   - [ ] Improve Resizing
- [ ] Add control features
   - [x] Play/Pause
   - [x] Fast Forward (Improve this)
   - [ ] Rewind
- [x] Improve error reporting
- [ ] Auto Installer
   
## Install

Currently ASCIIPlay is in development status with no installation candidate. If you want to use it you will need to get the [listed dependencies](#Dependencies) in order to compile

Download repository:

```
git clone https://github.com/aidancrowther/ASCIIPlay
```
   
Navigate to ASCIIPlay folder and prepare Makefile

```
cmake .
```
   
Make and run the executable

```
make
./telnetflix -f <filename> -s <subtitle file>
```
     
## Usage

After getting ASCIIPlay installed the available flags for the executable are:

```
-f             Specify an input video file (Required)
-s             Specify an input subtitle file (Optional)
-h             Display help menu
--slow-mode    Allow slower systems time to start reading the input file
--frame-rate   Specify a framerate (>1 && < src framerate) to render at
--no-render    Disable the renderer output (Don't recommend using this if you want to see anything)
--enable-skew  Disable frametime skew correction (Don't use this if you want subs to do anything useful)
--debug        Display debug information while rendering
```

## Problems

   Please feel free to notify me of any issues you encounter, and I will fix them as soon as possible. I am open to any suggestions or requests, and will work to make the program as functional as possible

### Dependencies

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
* libdc1394-dev

```
sudo apt install ffmpeg cmake make gcc build-essential pkg-config libgtk-2-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev gfortran openexr libatlas-base-dev python3-dev python3-numpy libtbb2 libtbb-dev libdc1394-dev
```
