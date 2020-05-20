#define MAX_BUFFER 100
#define NUM_THREADS 3
#define NUM_CHANNELS 3
#define CHAR_X 8
#define CHAR_Y 16
#define R 0
#define G 1
#define B 2

//#include "Lib/ascii_art.h"

#include <stdio.h>
#include <unistd.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <pthread.h>

#include <time.h>

struct videoStream {
	double fps;
	int width;
	int height;
	int time;
	char* subFile;
	struct vBuffer* buffer;
	volatile int bufferLength;
	volatile int semaphore;
};

struct vBuffer {
	uint8_t *frame;
	struct vBuffer *next;
	uint8_t streamEnd;
};

struct vStreamArgs {
	char *file;
	struct vBuffer *buffer;
};
