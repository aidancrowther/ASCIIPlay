#include "helpers.h"

// Convert an AVFrame to a B/W image
uint8_t* addPadding(AVFrame *pFrame, int width, int height){

	uint8_t *img = (uint8_t*) malloc(width*height * sizeof(uint8_t*));

	// Iterate over the frame buffer
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			// Convert the three channels to a grayscale representation
			*(img+x+y*width) = *(pFrame->data[0]+y*(pFrame->linesize[0])+(x/2));
		}
	}

	return img;
}

int getNiceFramerate(double framerate){
	int multiplier = 1;
	while(ceil(framerate*multiplier) != floor(framerate*multiplier)) multiplier *= 10;
	return multiplier;
}

// Convert time codes to seconds
double decodeTime(char* time, int last){

	// Determine offset based on which timecode we want
	int offset = 0;
	if (last) offset = 17;

	// Multiplication factors
	double factor[9] = {36000, 3600, 600, 60, 10, 1, 0.1, 0.01, 0.001};
	int index = 0;
	double result = 0.0;
	
	// Iterate over our timecode converting to seconds
	for(int i=offset; i<=offset+TIME_CODE_LENGTH; i++){
		if(*(time+i) == ':' || *(time+i) == ',') continue;
		result += (((int) *(time+i)) - '0') * factor[index++];
	}

	return result;

}

// Compare two timeval structs to determine difference
int timeval_subtract (result, x, y)
     struct timeval *result, *x, *y;
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "temp/frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

void updateFrameDimensions(){
    // Get rendering dimensions
	int x = 0, y = 0;
	getmaxyx(stdscr, y, x);

	int scr_x = x, scr_y = y;

	// Update our scaling if needed
	if(vStream.width == 0 && vStream.height == 0){
		vStream.height=y*CHAR_Y;
		vStream.width=x*CHAR_X;
		vStream.scr_width = vStream.width/CHAR_X;
		vStream.scr_height = vStream.height/CHAR_Y;
	} else if (scr_x != vStream.scr_width || scr_y != vStream.scr_height){
		vStream.scr_width = scr_x;
		vStream.scr_height = scr_y;
		vStream.height=y*CHAR_Y;
		vStream.width=x*CHAR_X;

		vStream.resized = true;

		// Clear buffer region
		clear();
	}
}