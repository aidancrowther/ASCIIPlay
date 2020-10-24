#ifndef CONSTANTS
#define CONSTANTS

#define ALIGNMENT 32
#define MAX_BUFFER 100
#define MAX_CHAR 50
#define NUM_ROWS 2
#define NUM_THREADS 3
#define NUM_CHANNELS 3
#define TIME_CODE_LENGTH 11
#define MATCH_EXPR "[0-9][0-9]:[0-9][0-9]:[0-9][0-9],[0-9][0-9][0-9] --> [0-9][0-9]:[0-9][0-9]:[0-9][0-9],[0-9][0-9][0-9]"
#define CHAR_X 8
#define CHAR_Y 8
#define R 0
#define G 1
#define B 2

#include "Lib/ascii_art.h"
#include <stdio.h>
#include <unistd.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <pthread.h>
#include <sys/time.h>
#include <ncurses.h>
#include <stdio.h>
#include <regex.h>
#include <math.h>

struct videoStream {
	double fps;
	double srcFps;
	int width;
	int scr_width;
	int height;
	int scr_height;
	int scale_x;
	int scale_y;
	double time;
	double realTime;
	int frameCount;
	char* subFile;
	double* subTimes;
	char subs[2][MAX_CHAR];
	int subPos;
	int fpPos;
	volatile int bufferLength;
	volatile int semaphore;
};

struct vBuffer {
	uint8_t *frame;
	struct vBuffer *next;
	double timestamp;
};

struct vStreamArgs {
	char *file;
	struct vBuffer *buffer;
};

struct playback {
	bool pause;
	bool fast_forward;
};

struct videoStream vStream;
struct timespec wait;
struct playback controls;

FILE* subFile;
regex_t matchTime;
int debug;
int skew;

#endif