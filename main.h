#ifndef MAIN
#define MAIN

#include "constants.h"
#include "helpers.h"
#include "renderer.h"
#include "controller.h"

// Open up the file data stream and parse frames
void openStream(struct vStreamArgs* args);

// Load array of subtitle time codes
void loadSubs();

// Write new frame data to the tail of the buffer queue
void writeBuffer(struct vBuffer **buffer, AVFrame *frame);

// Return frame buffer at the head of our queue
struct vBuffer* readBuffer(struct vBuffer **buffer);

// Run continuously in the background rendering frames when needed
void *renderingEngine(struct vBuffer *buffer);

// Spawn all neccessary threads and manage them
int main(int argc, char *argv[]);

#endif