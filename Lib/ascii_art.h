#ifndef __ASCII_ART__
#define __ASCII_ART__

#ifndef INDEX_MATRIX_SZ
#define INDEX_MATRIX_SZ 640 * 480
#endif

typedef struct ascii_render {
	int nGlyphs;
	unsigned char* zGlyphs[256];
	unsigned char zMatrix[INDEX_MATRIX_SZ];
	int nRows;
	int nCols;
	int* pTree;
}ascii_render;

void AsciiArtInit(ascii_render *pRender);
unsigned int AsciiArtTextBufSize(ascii_render *pRender, int img_width, int img_height);

// AsciiArtRender
// renders???
//
// ascii_render* pRender: initialized renderer struct
// unsigned char* zPixel: in/out var
// int* pnWidth: in/out var
// int* pnHeight: in/out var
// unsigned char* zBuf: optional output
// int Optimize:
void AsciiArtRender(ascii_render* pRender, unsigned char* zPixel, int* pnWidth, int* pnHeight, unsigned char* zBuf, int Optimize);

#endif

