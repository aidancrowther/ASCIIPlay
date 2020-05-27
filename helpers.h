#ifndef HELPERS
#define HELPERS

#include "constants.h"

// Convert an AVFrame to a B/W image
uint8_t* grayscale(AVFrame *pFrame, int width, int height);

// Convert time codes to seconds
double decodeTime(char* time, int last);

// Compare two timeval structs to determine difference
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);

int getNiceFramerate(double framerate);

uint8_t* reduceFrame(uint8_t *src, int width, int height, int factor);

#endif