#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#define clrscr() printf("\e[1;1H\e[2J")
#endif

#include "main.h"

// Open up the file data stream and parse frames
void openStream(struct vStreamArgs* args){

	char *filename = args->file;
	struct vBuffer *videoBuffer = args->buffer;

	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVFrame *pFrameRGB = NULL;

	double enableFramePacing = 0;
	int frameAdjustment = 0;

	// Open video file
	if(avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0)
		pthread_exit((void *)1); // Couldn't open file

	//Get stream info
	if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
  		pthread_exit((void *)2); // Couldn't find stream information
	
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

	if(vStream.fps == 0.0){
		vStream.fps = (double) pFormatCtx->streams[stream]->avg_frame_rate.num / (double) pFormatCtx->streams[stream]->avg_frame_rate.den;
		vStream.srcFps = vStream.fps;
	}
	else{
		enableFramePacing = (double) pFormatCtx->streams[stream]->avg_frame_rate.num / (double) pFormatCtx->streams[stream]->avg_frame_rate.den;
		frameAdjustment = getNiceFramerate(enableFramePacing);
		vStream.srcFps = enableFramePacing;
	}
	vStream.height = pFormatCtx->streams[stream]->codecpar->height;
	vStream.width = pFormatCtx->streams[stream]->codecpar->width*2;

	if (stream == -1) pthread_exit((void *)3); // No video streams found

	//Try to find the video codec
	pCodec = avcodec_find_decoder(pFormatCtx->streams[stream]->codecpar->codec_id);
	if (pCodec == NULL) pthread_exit((void *)4); // Unsupported codec

	// Copy the found codec
	pCodecCtx = avcodec_alloc_context3(pCodec);

	// Copy codec parameters and open the context
	avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[stream]->codecpar);
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) pthread_exit((void *)5); //Could not open codec

	// Alocate a frame
	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	if (pFrame == NULL) pthread_exit((void *)6); //Could not allocate AVFrame
	if (pFrameRGB == NULL) pthread_exit((void *)7);

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

	for(int frame=0; av_read_frame(pFormatCtx, &packet) >= 0; frame++){
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

				vStream.frameCount++;
				vStream.realTime = (vStream.frameCount-(vStream.bufferLength*(vStream.srcFps/vStream.fps)))/vStream.srcFps;
				if (enableFramePacing && ((frame)%(int) ((ceil(enableFramePacing))/(vStream.fps)))) continue;
				// Wait until there is room in the frame buffer
				while(vStream.bufferLength >= MAX_BUFFER) nanosleep(&wait, (struct timespec*) NULL);
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

	pthread_exit((void *)0);
}

// Write new frame data to the tail of the buffer queue
void writeBuffer(struct vBuffer **buffer, AVFrame *frame){
	struct vBuffer *newBuffer;

	// Wait for the semaphore
	while (vStream.semaphore != 0) nanosleep(&wait, (struct timespec*) NULL);;
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
	vStream.bufferLength++;

	vStream.semaphore = 0;
}

// Return frame buffer at the head of our queue
struct vBuffer* readBuffer(struct vBuffer **buffer){
	struct vBuffer *nextBuffer;

	// Wait for the semaphore so we don't cause any issues
	while (vStream.semaphore != 0) nanosleep(&wait, (struct timespec*) NULL);;
	vStream.semaphore = 1;

	// Update buffer and queue
	nextBuffer = (*buffer)->next;
	free((*buffer)->frame);
	free(*buffer);
	vStream.bufferLength--;
	vStream.semaphore = 0;

	return &(*nextBuffer);

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

	if (vStream.subFile != NULL) loadSubs();

	// Timing variables
	struct timeval start, curr, diff;
	gettimeofday(&start, 0x0);
	int frameDigits = 0;

	// Run until we hit the streamEnd frame
	while(1){

		// Do not render when paused (sleep thread to not spin it)
		while(controls.pause) sleep(0.1);

		// Thread may be ready before the frames are, so wait for frames
		while(vStream.bufferLength <= 0) nanosleep(&wait, (struct timespec*) NULL);

		if (frameDigits == 0) frameDigits = getNiceFramerate(vStream.srcFps);

		// Update time tracking
		gettimeofday(&curr, 0x0);
		timeval_subtract(&diff, &curr, &start);

		// If enough time has passed between the last frame and this one render it
		// Alternatively, if we are fast forwarding loading frames as they come into the buffer
		if (diff.tv_usec >= (1/vStream.fps*1000000) || controls.fast_forward){
			// Update our timing variable
			gettimeofday(&start, 0x0);
			// Pull the new frame data from the buffer
			while(buffer->next == NULL);
			buffer = readBuffer(&buffer);
			vStream.time = vStream.time + (1/(vStream.fps));

			// Account for possible frame skew when rendering at different framerates
			if (!skew && (vStream.time > 1.02*vStream.realTime || vStream.time < 0.98*vStream.realTime)) vStream.time = vStream.realTime;

			// Pass frame data to renderer
			renderFrame(buffer->frame, vStream.width, vStream.height);

			if (buffer->next == NULL) break;

		}

		nanosleep(&wait, (struct timespec*) NULL);
	}

	// Close the subtitle file
	if (vStream.subFile != NULL) fclose(subFile);

	// Free memory
	free(buffer);
	pthread_exit((void *)0);
}

// Spawn all neccessary threads and manage them
int main(int argc, char *argv[]){

	char* filename = NULL;
	char* subfile = NULL;
	char* flag;
	int slowMode = 0;
	int disableRenderer = 0;

	// Initialize our control structure
	controls.pause = false;
	controls.fast_forward = false;

	wait.tv_nsec = 1000000;

	// Initialize our video buffer
	struct vBuffer *videoBuffer = (struct vBuffer*) malloc(sizeof(struct videoBuffer*));
	videoBuffer->frame = NULL;
	videoBuffer->next = NULL;

	// Initialize the parameters of our video stream
	vStream.bufferLength = 0;
	vStream.time = 0;
	vStream.subTimes = NULL;
	vStream.fpPos = 0;
	vStream.subPos = 0;
	vStream.fps = 0.0;
	vStream.scale_x = 0;
	vStream.scale_y = 0;

	// Setup thread tracking
	pthread_t threads[NUM_THREADS];

	for (int i=1; i<argc; i++){
		flag = *(argv+i);
		
		if (!strcmp(flag, "-f")){
			i++;
			filename = malloc(sizeof(char) * strlen(*(argv+i)));
			filename = *(argv+i);
		}

		if (!strcmp(flag, "-s")){
			i++;
			subfile = malloc(sizeof(char) * strlen(*(argv+i)));
			subfile = *(argv+i);
			vStream.subFile = subfile;\
		}

		if (!strcmp(flag, "--slow-mode")) slowMode = 1;

		if (!strcmp(flag, "--no-render")) disableRenderer = 1;

		if (!strcmp(flag, "--debug")) debug = 1;

		if (!strcmp(flag, "--enable-skew")) skew = 1;

		if (!strcmp(flag, "--frame-rate")){
			i++;
			if (strlen(*(argv+i)) > 1) vStream.fps = (((int) *(*(argv+i))-'0')*10) + ((int) (*(*(argv+i)+1)-'0'));
			else vStream.fps = ((int) (*(*(argv+i))-'0'));
		}

		if (!strcmp(flag, "-h")){
			printf("Usage ./telnetflix -f <FILENAME> [ -s <SUBFILE> ]\n");
			return 0;
		}
	}

	if (filename == NULL){
		printf("Error, no file specified\n");
		return -1;
	}

	// Initialize display
	if (!disableRenderer){
		initscr();
		noecho();
		curs_set(FALSE);
	}

	// Setup regex
	regcomp(&matchTime, MATCH_EXPR , 0);

	// Register codecs
	av_register_all();

	// Prepare data for the stream
	struct vStreamArgs *vArgs = (struct vStreamArgs*) malloc(sizeof(struct vStreamArgs*));
	vArgs->file = filename;
	vArgs->buffer = videoBuffer;

	// Create and track our threads
	pthread_create(&threads[0], NULL, (void * (*)(void *))openStream, vArgs);
	if (slowMode) sleep(4);
	pthread_create(&threads[1], NULL, (void * (*)(void *))renderingEngine, videoBuffer);
	pthread_create(&threads[2], NULL, inputHandler, NULL);
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
	pthread_cancel(threads[2]);

	// Free allocated memory
	free(vArgs);
	if(vStream.subTimes != NULL) free(vStream.subTimes);

	// Terminate our screen
	endwin();

	return 0;

}