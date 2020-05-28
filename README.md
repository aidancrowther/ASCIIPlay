# ASCIIPlay

Description:

  ASCIIPlay is a video player designed for the purpouse of rendering to ASCII any video file given to it, inspired by towel.blinkenlights.nl. The reasoning for my making my  own ASCII video player is due to the lack of an existing ASCII renderer with support for subtitle or audio playback. Thus I hope for  ASCIIPlay to develop to fill this niche. A server is currently publicly accessible to view an example of this project, and can be accessed using: `ssh anonymous@telnetflix.com -p 20`
  
  ![](ASCIIPlay.gif)
  
Goals:

  My main goal for ASCIIPlay is to develop a fully functional video player with a variety of features, while also practicing my C development and trying features I haven't previously used much. To this regard, I have definitely learned much more about data manipulation and threading in C programming, and I hope to expand into more practice as the project moves forward.
  
Status:
  
   ASCIIPlay is currently stable, with many features being planned and added frequently.
   
   Currently Supports:
   
   - Reading in a variety of video formats such ash mp4, mkv, ...
   - Subtitle rendering using SRT formatted subtitles
   - Frame limiting for lower end systems or situations where lower bitrate is necessary
   - Black and White ASCII rendering only at this time
   
Roadmap:

   - [x] Load and read video files
   - [x] Render ASCII representation
   - [x] Implement frame buffer
   - [x] Make use of threading
   - [x] Add subtitle support
   - [x] Performance improvements
   - [x] framerate limiting
   - [ ] Use custom ASCII renderer to remove closed source dependencies
   - [ ] More subtitle filetypes
   - [ ] Alternate subtitle rendering modes
   - [x] Improve error reporting
   - [ ] Auto Installer
   - [ ] Colour support
   - [x] Audio renderer
   
Requirements:

    - Currently ASCIIPlay is in development status with no installation candidate, if you want to use it you will need to get the dependencies listed in order to compile
   
Install:

   - Download repository
     `git clone https://github.com/aidancrowther/Odysseus`
     
   - Navigate to ASCIIPlay folder and prepare Makefile
     `cmake .`
     
   - Make and run the executable
     `make`
     `./telnetflix -f <filename> -s <subtitle file>`
     
Usage:

   After getting ASCIIPlay installed the available flags for the executable are:
   
   - __-f__: Specify an input video file (Required)
   - __-s__: Specify an input subtitle file (Optional)
   - __-h__: Display help menu
   - __--slow-mode__: Allow slower systems time to start reading the input file
   - __--frame-rate__: Specify a framerate (>1 && < src framerate) to render at
   - __--no-render__: Disable the renderer output (for testing purposes only)
   - __--enable-skew__: Disable frametime skew correction (for testing purposes only)
   - __--debug__: Display debug information while rendering
      
Problems:

   Please feel free to notify me of any issues you encounter, and I will fix them as soon as possible. I am open to any suggestions or requests, and will work to make the program as functional as possible

Dependencies

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
