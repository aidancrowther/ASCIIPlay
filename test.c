#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#define clrscr() printf("\e[1;1H\e[2J")
#endif

#include "test.h"

struct videoStream vStream;
WINDOW *vWindow;

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
				writeBuffer(videoBuffer, pFrameRGB, 0);

			}
		}

		// Free the packet we allocated
		av_free_packet(&packet);
	}

	av_free_packet(&packet);

	// Free the RGB image
	av_free(buffer);
	av_free(pFrameRGB);
	writeBuffer(videoBuffer, NULL, 1);

	// Free the YUV frame
	av_free(pFrame);

	// Close the codecs
	avcodec_close(pCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);

	pthread_exit(0);
}

void renderFrame(uint8_t *img, int width, int height) {
	//ascii_render sRender;

	unsigned char *zText;
	unsigned int nBytes;
	
	/*AsciiArtInit(&sRender);

	// Allocate space for ascii frame
	nBytes = AsciiArtTextBufSize(&sRender, width, height);
	zText = malloc(nBytes);

	// Call ASCII renderer
	AsciiArtRender(&sRender, img, &width, &height, zText, 1);

	// Generate subtitles
	generateSubs(&zText);

	width = width/CHAR_X;
	height = height/CHAR_Y;

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
		mvaddch(i/width, i%width, stripped[i]);
	}
	refresh();

	// Free allocated memory
	free(zText);*/

	return;
}

// Write new frame data to the tail of the buffer queue
void writeBuffer(struct vBuffer *buffer, AVFrame *frame, uint8_t last){
	struct vBuffer *newBuffer;

	// Wait for the semaphore
	while (vStream.semaphore != 0);
	vStream.semaphore = 1;

	// Find the ned of the buffer
	while (buffer->next != NULL) buffer = buffer->next;
	// Allocate the buffer
	newBuffer = (struct vBuffer*) malloc(sizeof(struct videoBuffer*)+sizeof(uint8_t*) * vStream.width*vStream.height);
	
	// Handle the NULL head frame buffer
	if (frame != NULL){
		// Convert frame data to a grayscale represenation and store it
		uint8_t *img = grayscale(frame, vStream.width, vStream.height);
		newBuffer->frame = img;
	}

	// Update frame buffer
	buffer->next = newBuffer;
	newBuffer->next = NULL;
	newBuffer->streamEnd = last;
	vStream.bufferLength++;

	vStream.semaphore = 0;
}

// Return frame buffer at the head of our queue
struct vBuffer* readBuffer(struct vBuffer *buffer){
	struct vBuffer *prevBuffer;

	// Wait for the semaphore so we don't cause any issues
	while (vStream.semaphore != 0);
	vStream.semaphore = 1;

	// Skip the head NULL buffer
	while (buffer->next == NULL){
		// Store/Free the old buffer and get the new one
		prevBuffer = buffer;
		buffer = buffer->next;
		free(prevBuffer->frame);
		free(prevBuffer);
	}

	// Update buffer and queue
	buffer = buffer->next;
	vStream.bufferLength--;
	vStream.semaphore = 0;

	return buffer;

}

// Run continuously in the background rendering frames when needed
void *renderingEngine(struct vBuffer *buffer){

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
			buffer = readBuffer(buffer);
			// Exit on streamEnd
			if(buffer->streamEnd) break;
			// Pass frame data to renderer
			renderFrame(buffer->frame, vStream.width, vStream.height);
		}

	}

	free(buffer);

	pthread_exit(0);
}

// Superimpose subtitles over the generated ASCII
void generateSubs(char **frame){

	char tmp[10] = "ooga booga";

	for (int i=0; i<30; i++){
		//*(*(frame)+i) = '0';
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

	//initscr();
	//noecho();
	//curs_set(FALSE);

	struct vBuffer *videoBuffer = (struct vBuffer*) malloc(sizeof(struct videoBuffer*));
	videoBuffer->frame = NULL;
	videoBuffer->next = NULL;

	vStream.bufferLength = 0;
	vStream.time = 0;
	vStream.subFile = "sub.srt";

	pthread_t threads[NUM_THREADS];

	av_register_all();

	struct vStreamArgs *vArgs = (struct vStreamArgs*) malloc(sizeof(struct vStreamArgs*));

	vArgs->file = "shrek.mp4";
	vArgs->buffer = videoBuffer;

	pthread_create(&threads[0], NULL, openStream, vArgs);
	pthread_create(&threads[1], NULL, renderingEngine, videoBuffer);
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	free(vArgs);

	//endwin();

	return 0;

}