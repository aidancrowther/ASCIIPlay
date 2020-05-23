#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#define clrscr() printf("\e[1;1H\e[2J")
#endif

#include "test.h"

struct videoStream vStream;
FILE* subFile;
regex_t matchTime;

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

void openStream(struct vStreamArgs* args){

	char *filename = args->file;
	struct vBuffer *videoBuffer = args->buffer;

	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVFrame *pFrameRGB = NULL;

	// Open video file
	if(avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0)
		pthread_exit(1); // Couldn't open file

	//Get stream info
	if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
  		pthread_exit(2); // Couldn't find stream information
	
	// Find first video stream
	int i = 0;
	int stream = -1;

	// Walk over all streams until we find the first video stream
	for(int i=0; i<pFormatCtx->nb_streams; i++){
		if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			stream = i;
			break;
		}
	}

	vStream.fps = (double) pFormatCtx->streams[stream]->avg_frame_rate.num / (double) pFormatCtx->streams[stream]->avg_frame_rate.den;
	vStream.height = pFormatCtx->streams[stream]->codecpar->height;
	vStream.width = pFormatCtx->streams[stream]->codecpar->width*2;

	if (stream == -1) pthread_exit(3); // No video streams found

	//Try to find the video codec
	pCodec = avcodec_find_decoder(pFormatCtx->streams[stream]->codecpar->codec_id);
	if (pCodec == NULL) pthread_exit(4); // Unsupported codec

	// Copy the found codec
	pCodecCtx = avcodec_alloc_context3(pCodec);

	// Copy codec parameters and open the context
	avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[stream]->codecpar);
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) pthread_exit(5); //Could not open codec

	// Alocate a frame
	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	if (pFrame == NULL) pthread_exit(6); //Could not allocate AVFrame
	if (pFrameRGB == NULL) pthread_exit(7);

	// Prepare a buffer for frame conversion
	int numBytes;
	uint8_t *buffer = NULL;

	numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

	struct SwsContext *sws_ctx = NULL;
	int frameFinished;
	AVPacket packet;

	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		AV_PIX_FMT_RGB24,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
	);

	while(av_read_frame(pFormatCtx, &packet) >= 0){
		// Check that the packet is from our stream
		if (packet.stream_index == stream){
			// Decode frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			// Check that this is a video frame
			if (frameFinished){
				// Convert frame to RGB image
				sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
					pFrame->linesize, 0, pCodecCtx->height,
					pFrameRGB->data, pFrameRGB->linesize);

				// Wait until there is room in the frame buffer
				while(vStream.bufferLength >= MAX_BUFFER);
				// Write the new frame to the buffer
				writeBuffer(&videoBuffer, pFrameRGB);

			}
		}

		// Free the packet we allocated
		av_free_packet(&packet);
	}

	// Free the RGB image
	av_free(buffer);
	//writeBuffer(videoBuffer, pFrameRGB, 1);
	av_free(pFrameRGB);

	// Free the YUV frame
	av_free(pFrame);

	// Close the codecs
	avcodec_close(pCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);

	pthread_exit(0);
}

void renderFrame(uint8_t *img, int width, int height) {
	ascii_render sRender;

	unsigned char *zText;
	unsigned int nBytes;
	
	AsciiArtInit(&sRender);

	// Allocate space for ascii frame
	nBytes = AsciiArtTextBufSize(&sRender, width, height);
	zText = malloc(nBytes);

	// Call ASCII renderer
	AsciiArtRender(&sRender, img, &width, &height, zText, 1);

	// Generate subtitles
	generateSubs(&zText);

	int x = 0, y = 0;
	getmaxyx(stdscr, y, x);

	width = width/CHAR_X;
	height = height/CHAR_Y;

	// Determine our text offset to center the screen
	x = (x-width)/2;
	y = (y-height)/2+(y-height)%2;

	int newBytes = nBytes-height;
	unsigned char stripped[newBytes];
	
	// Strip newline characters from the output stream
	int count = 0;
	for (int i = 0; i<nBytes; i++){
		if (*(zText+i) != '\n'){
			stripped[count++] = *(zText+i);
		}
	}

	// Print ASCII to screen
	for (int i=0; i<newBytes; i++){
		mvaddch(y+i/width, x+i%width, stripped[i]);
	}
	refresh();

	// Free allocated memory
	free(zText);

	return;
}

// Write new frame data to the tail of the buffer queue
void writeBuffer(struct vBuffer **buffer, AVFrame *frame){
	struct vBuffer *newBuffer;

	// Wait for the semaphore
	while (vStream.semaphore != 0);
	vStream.semaphore = 1;

	// Find the ned of the buffer
	while ((*buffer)->next != NULL) (*buffer) = (*buffer)->next;
	// Allocate the buffer
	(*buffer)->next = (struct vBuffer*) malloc(sizeof(struct videoBuffer*)+sizeof(uint8_t*) * vStream.width*vStream.height);
	
	// Handle the NULL head frame buffer
	if (frame != NULL){
		// Convert frame data to a grayscale represenation and store it
		(*buffer)->next->frame = grayscale(frame, vStream.width, vStream.height);
	}

	// Update frame buffer
	(*buffer)->next->next = NULL;
	(*buffer)->next->time = (*buffer)->time + (1/vStream.fps);
	vStream.bufferLength++;

	vStream.semaphore = 0;
}

// Return frame buffer at the head of our queue
struct vBuffer** readBuffer(struct vBuffer **buffer){
	struct vBuffer *nextBuffer;

	// Wait for the semaphore so we don't cause any issues
	while (vStream.semaphore != 0);
	vStream.semaphore = 1;

	// Update buffer and queue
	//if((*buffer)->time >= 2.0) sleep(5);
	nextBuffer = (*buffer)->next;
	free((*buffer)->frame);
	free(*buffer);
	vStream.bufferLength--;
	vStream.semaphore = 0;

	return &(*nextBuffer);

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

// Load array of subtitle time codes
void loadSubs(){

	char buffer[MAX_CHAR];

	// Open our file for reading
	subFile = fopen(vStream.subFile, "r");

	// Return if no file provided
	if (!subFile) return;

	int lines = 0;

	// Determine how much space we need for storing data
	while(fgets(buffer, sizeof(buffer), subFile) != NULL){
		if(!regexec(&matchTime, buffer, 0, NULL, 0)) lines++;
	}

	// Reset file pointer
	rewind(subFile);

	// Allocate space for our sub data
	vStream.subTimes = malloc(sizeof(double) * lines * 3);
	int count = 0;
	lines = 0;

	// Write our subtitle data
	// Format [start time, end time, first line of text, ...]
	while(fgets(buffer, sizeof(buffer), subFile) != NULL){
		if(!regexec(&matchTime, buffer, 0, NULL, 0)){
			*(vStream.subTimes+(count++)) = decodeTime(buffer, 0);
			*(vStream.subTimes+(count++)) = decodeTime(buffer, 1);
			*(vStream.subTimes+(count++)) = lines+1;
		}
		lines++;
	}

	// Reset file pointer
	rewind(subFile);

}

// Run continuously in the background rendering frames when needed
void *renderingEngine(struct vBuffer *buffer){

	loadSubs();

	// Timing variables
	struct timeval start, curr, diff;
	gettimeofday(&start, 0x0);

	// Run until we hit the streamEnd frame
	while(1){

		// Thread may be ready before the frames are, so wait for frames
		while(vStream.bufferLength <= 0);

		// Update time tracking
		gettimeofday(&curr, 0x0);
		timeval_subtract(&diff, &curr, &start);

		// If enough time has passed between the last frame and this one render it
		if (diff.tv_usec >= (1/vStream.fps)*1000000){
			// Update our timing variable
			gettimeofday(&start, 0x0);
			// Pull the new frame data from the buffer
			while(buffer->next == NULL);
			buffer = readBuffer(&buffer);
			vStream.time = buffer->time;
			// Pass frame data to renderer
			renderFrame(buffer->frame, vStream.width, vStream.height);

			if(buffer->next == NULL) break;
		}
	}

	// Free memory
	free(buffer);
	fclose(subFile);
	pthread_exit(0);
}

// Strip unwanted characters from strings and center them
void prepSubs(char *str){

	char newString[MAX_CHAR];
	int iterator = 0;

	// Strip newline and quotes
	for(int i=0; i<MAX_CHAR; i++){
		if (*(str+i) != '\n' && *(str+i) != '"') newString[iterator++] = *(str+i);
	}

	// Determine center offset
	int length = strlen(newString);
	int offset = (MAX_CHAR-length)/2;
	iterator = 0;

	// Update string to centered and stripped text
	for (int i=0; i<MAX_CHAR; i++){
		*(str+i) = ' ';
		if (i >= offset-1 && i < offset+length-2) *(str+i) = newString[iterator++];
	}

}

// Superimpose subtitles over the generated ASCII
void generateSubs(char **frame){

	char buffer[MAX_CHAR];
	double start = *(vStream.subTimes+vStream.subPos*3+0);
	double end = *(vStream.subTimes+vStream.subPos*3+1);
	int currLine = (int) *(vStream.subTimes+vStream.subPos*3+2);
	int reading = 0;

	// Initialize the subtitles
	if (vStream.subPos == 0){
		if(vStream.fpPos == 0){
			// Iterate over the subtitles until we find what we want
			while(fgets(buffer, sizeof(buffer), subFile) != NULL){
				// Copy the two subtitle strings
				if (vStream.fpPos == currLine){
					prepSubs(buffer);
					strcpy(vStream.subs[0], buffer);
					reading = 1;
				} else if (reading){
					prepSubs(buffer);
					strcpy(vStream.subs[1], buffer);
					vStream.fpPos++;
					reading = 0;
					break;
				}
				vStream.fpPos++;
			}
		}
	}

	// Render our subtitles between start and stop times
	if (vStream.time >= start && vStream.time < end){

		// Total display width is a function of character size
		int width = vStream.width/CHAR_X;
		int height = vStream.height/CHAR_Y;

		// Calculate bounding box for our subtitles
		int tBox_X = MAX_CHAR+2, tBox_Y = NUM_ROWS+2;
		int bLoc_X = (width/2) - (tBox_X/2), bLoc_Y = (height-2) - tBox_Y;

		// Tracker variables
		int x_Pos = 0;
		int y_Pos = 0;

		// Render our text box
		for (int y=0; y<height; y++){
			for (int x=0; x<width+1; x++){
				// Fill box with blank characters
				if ((x >= bLoc_X && y >= bLoc_Y) && (x < bLoc_X+tBox_X && y < bLoc_Y+tBox_Y)) *(*frame+(x+(y*(width+1)))) = ' ';
				// Render text within the bounding box
				if ((x >= bLoc_X+1 && y >= bLoc_Y+1) && (x < bLoc_X+tBox_X-1 && y < bLoc_Y+tBox_Y-1)) *(*frame+(x+(y*(width+1)))) = vStream.subs[y_Pos++/(tBox_X-2)][x_Pos++%(tBox_X-2)];
			}
		}

	}
	// Update our file pointer and subtitles
	else if (vStream.time >= end){
		vStream.subPos++;
		currLine = (int) *(vStream.subTimes+(vStream.subPos)*3+2);
		if (vStream.fpPos <= currLine+1){
			while(fgets(buffer, sizeof(buffer), subFile) != NULL){
				if (vStream.fpPos == currLine){
					prepSubs(buffer);
					strcpy(vStream.subs[0], buffer);
					reading = 1;
				} else if (reading){
					prepSubs(buffer);
					strcpy(vStream.subs[1], buffer);
					vStream.fpPos++;
					reading = 0;
					break;
				}
				vStream.fpPos++;
			}
		}
	}

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

// Spawn all neccessary threads and manage them
int main(int argc, char *argv[]){

	// Initialize display
	initscr();
	noecho();
	curs_set(FALSE);

	// Setup regex
	regcomp(&matchTime, MATCH_EXPR , 0);

	// Initialize our video buffer
	struct vBuffer *videoBuffer = (struct vBuffer*) malloc(sizeof(struct videoBuffer*));
	videoBuffer->frame = NULL;
	videoBuffer->next = NULL;
	videoBuffer->time = 0;

	// Initialize the parameters of our video stream
	vStream.bufferLength = 0;
	vStream.time = 0;
	vStream.subFile = "sub.srt";
	vStream.subTimes = NULL;
	vStream.fpPos = 0;
	vStream.subPos = 0;

	// Setup thread tracking
	pthread_t threads[NUM_THREADS];

	// Register codecs
	av_register_all();

	// Prepare data for the stream
	struct vStreamArgs *vArgs = (struct vStreamArgs*) malloc(sizeof(struct vStreamArgs*));
	vArgs->file = "shrek.mp4";
	vArgs->buffer = videoBuffer;

	// Create and track our threads
	pthread_create(&threads[0], NULL, openStream, vArgs);
	pthread_create(&threads[1], NULL, renderingEngine, videoBuffer);
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	// Free allocated memory
	free(vArgs);
	if(vStream.subTimes != NULL) free(vStream.subTimes);

	// Terminate our screen
	endwin();

	return 0;

}