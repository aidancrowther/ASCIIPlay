#include "renderer.h"

void renderFrame(uint8_t *img, int width, int height) {
	ascii_render sRender;

	unsigned char *zText;
	unsigned int nBytes;

	int x = 0, y = 0;
	getmaxyx(stdscr, y, x);

	int scr_x = x, scr_y = y;

	if(vStream.scr_width == 0 && vStream.scr_height == 0){
		vStream.scr_width = scr_x;
		vStream.scr_height = scr_y;
	} else if (scr_x != vStream.scr_width || scr_y != vStream.scr_height){
		vStream.scr_width = scr_x;
		vStream.scr_height = scr_y;

		// Clear buffer region
		clear();

		int scale = 1;
		while((width/CHAR_X)/scale > scr_x || (height/CHAR_Y)/scale > scr_y) scale++;
		vStream.render_scale = scale;
	}

	if(vStream.render_scale > 1){
		resizeFrame(img, width, height);
	}

	width = width/vStream.render_scale, height = height/vStream.render_scale;

	AsciiArtInit(&sRender);

	// Allocate space for ascii frame
	nBytes = AsciiArtTextBufSize(&sRender, width, height);
	zText = malloc(nBytes);

	// Call ASCII renderer
	AsciiArtRender(&sRender, img, &width, &height, zText, 1);

	// Generate subtitles
	if(vStream.subFile != NULL) generateSubs(&zText);

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

	if(debug){
		char *timecode[20];
		char *realtime[20];
		char *nextSubStart[MAX_CHAR];
		char *nextSubEnd[MAX_CHAR];
		char *nextSubLine[MAX_CHAR];
		char *windowSize[20];
		char *rendererSize[MAX_CHAR];
		sprintf(timecode, "Time: %f", vStream.time);
		sprintf(realtime, "Time: %f", vStream.realTime);
		sprintf(nextSubStart, "Next sub: %f", *(vStream.subTimes+vStream.subPos*3+0));
		sprintf(nextSubEnd, "Sub Ends: %f Sub File ptr: %d", *(vStream.subTimes+vStream.subPos*3+1), vStream.fpPos);
		sprintf(nextSubLine, "Sub Line: %f Array idx: %d", *(vStream.subTimes+vStream.subPos*3+2), vStream.subPos);
		sprintf(windowSize, "Width: %d Height: %d", scr_x, scr_y);
		sprintf(rendererSize, "R_Width: %d R_Height: %d Scale: %d", width, height, vStream.render_scale);
		for (int i=0; i<newBytes; i++){
			if(i%width < strlen(timecode) && i/width == 0) mvprintw(y, x, timecode);
			if(i%width < strlen(realtime) && i/width == 1) mvprintw(y+1, x, realtime);
			if(i%width < strlen(nextSubStart) && i/width == 2) mvprintw(y+2, x, nextSubStart);
			if(i%width < strlen(nextSubEnd) && i/width == 3) mvprintw(y+3, x, nextSubEnd);
			if(i%width < strlen(nextSubLine) && i/width == 4) mvprintw(y+4, x, nextSubLine);
			if(i%width < strlen(windowSize) && i/width == 5) mvprintw(y+5, x, windowSize);
			if(i%width < strlen(rendererSize) && i/width == 6) mvprintw(y+6, x, rendererSize);
		}
	}

	refresh();

	// Free allocated memory
	free(zText);

	return;
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

	// Declare the end of our string
	*(str+MAX_CHAR) = '\0';

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
		int subPos = vStream.subPos;
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
		vStream.subPos = subPos;
	}

}