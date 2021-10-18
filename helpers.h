#ifndef HELPERS
#define HELPERS

#include "constants.h"

// Convert an AVFrame to a B/W image
uint8_t* grayscale(AVFrame *pFrame, int width, int height);

// Convert time codes to seconds
double decodeTime(char* time, int last);

// Resize a given frame by the specified scale factor
void resizeFrame(uint8_t *img, int width, int height);

void betterScaler(uint8_t *img, int width, int height);

// Compare two timeval structs to determine difference
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);

int getNiceFramerate(double framerate);

#endif