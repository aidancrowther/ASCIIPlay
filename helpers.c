#include "helpers.h"

// Convert an AVFrame to a B/W image
uint8_t* grayscale(AVFrame *pFrame, int width, int height){
	uint8_t *img = (uint8_t*) malloc(width*height * sizeof(uint8_t*));
	uint8_t pixel[NUM_CHANNELS];

	// Iterate over the frame buffer
	for(int y = 0; y < height; y++){
		for(int x = 0; x < width; x++){
			/*
			for(int channel=0; channel < NUM_CHANNELS; channel++){
				// Capture pixel data for each channel
				pixel[channel] = *(pFrame->data[0]+y*(pFrame->linesize[0])+(x/2)+channel);
			}
			*/
			// Convert the three channels to a grayscale representation
			*(img+(x)+y*width) = *(pFrame->data[0]+y*(pFrame->linesize[0])+(x/2));
		}
	}

	return img;
}

void resizeFrame(uint8_t *img, int width, int height){
	/*
	// The destination dimensions of our array
	int new_width = width - vStream.scale_x*CHAR_X;
	int new_height = height - vStream.scale_y*CHAR_Y;
	
	// Determine the interval at which we need to drop lines
	int skip_x = width / (width - new_width);
	int skip_y = height / (height - new_height);

	int loc = 0;

	// Scale y axis to the new dimensions
	for(int j=0; j<height; j++){
		if(j%skip_y == 0){
			loc++;
			continue;
		}
		for(int i=0; i<width; i++){
			*(img+(i+(j-loc)*width)) = *(img+(i+j*width));
		}
	}

	loc = 0;

	// Scale x axis to the new dimensions
	for(int i=0; i<(height*width); i++){
		if(i%skip_x == 0){
			loc++;
			continue;
		}
		*(img+(i-loc)) = *(img+(i));
	}
	*/
}

void betterScaler(uint8_t *img, int width, int height){

	int scale_factor = vStream.scale;
	int skipped = scale_factor - 1;

	int loc = 0;
	int count = 0;

	// Scale y axis to the new dimensions
	for(int j=0; j<height; j++){
		if(count > 0){
			loc++;
			if(count++ >= skipped) count = 0;
			continue;
		}
		count++;
		for(int i=0; i<width; i++){
			*(img+(i+(j-loc)*width)) = *(img+(i+j*width));
		}
	}

	loc = 0;
	count = 0;

	// Scale x axis to the new dimensions
	for(int i=0; i<(height*width); i++){
		if(count > 0){
			loc++;
			if(count++ >= skipped) count = 0;
			continue;
		}
		count++;
		*(img+(i-loc)) = *(img+(i));
	}
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