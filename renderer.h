#ifndef RENDERER
#define RENDERER

#include "constants.h"

// Render the frame to ASCII
void renderFrame(uint8_t *img, int width, int height);

// Strip unwanted characters from strings and center them
void prepSubs(char *str);

// Superimpose subtitles over the generated ASCII
void generateSubs(char **frame);

// Resize a given frame by the specified scale factor
void resizeFrame(uint8_t *img, int width, int height);

#endif