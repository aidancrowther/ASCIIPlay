#include "helpers.h"

// Convert an AVFrame to a B/W image
uint8_t* grayscale(AVFrame *pFrame, int width, int height){
	uint8_t *img = (uint8_t*) malloc(width*height * sizeof(uint8_t*));
	uint8_t pixel[NUM_CHANNELS];

	// Iterate over the frame buffer
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width*3; x += 3){
			for(int channel=0; channel < NUM_CHANNELS; channel++){
				// Capture pixel data for each channel
				pixel[channel] = *(pFrame->data[0]+y*(pFrame->linesize[0])+(x/2)+channel);
			}
			// Convert the three channels to a grayscale representation
			*(img+(x/3)+y*width) = (pixel[R] + pixel[G] + pixel[B])/NUM_CHANNELS;
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