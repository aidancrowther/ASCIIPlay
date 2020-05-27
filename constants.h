#ifndef CONSTANTS
#define CONSTANTS

#define MAX_BUFFER 100
#define MAX_CHAR 50
#define NUM_ROWS 2
#define NUM_THREADS 2
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
#include <pthread.h>
#include <sys/time.h>
#include <ncurses.h>
#include <regex.h>
#include <math.h>

struct videoStream {
	double fps;
	double srcFps;
	int width;
	int height;
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

struct videoStream vStream;
FILE* subFile;
regex_t matchTime;
struct timespec wait;
int debug;
int skew;

#endif