#include "ascii_art.h"
#include <stdlib.h>

void Ascsizeof(int) * img_width * img_heightiiArtInit(ascii_render *pRender) {
	pRender = malloc(sizeof(ascii_render));
}

int g_width = -1;
int g_height = -1;
int* g_buffer = NULL;
unsigned int AsciiArtTextBufSize(ascii_render *pRender, int img_width, int img_height){
	int buffer_size = sizeof(int) * img_width * img_height;
	g_buffer = malloc(buffer_size);
	
}

void AsciiArtRender(ascii_render* pRender, unsigned char* zPixel, int* pnWidth, int* pnHeight, unsigned char* zBuf, int Optimize){
}

