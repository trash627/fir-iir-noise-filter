#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <sndfile.h>
#include <math.h>

#define PI 3.14159265
#define Fs 48000
#define f 2400

int main(int argc, char *argv[])
{

	//Require 2 arguments: input file and output file
	if(argc < 3)
	{
		printf("Not enough arguments \n");
		return -1;
	}

	SF_INFO sndInfo;
	SNDFILE *sndFile = sf_open(argv[1], SFM_READ, &sndInfo);
	if (sndFile == NULL) {
		fprintf(stderr, "Error reading source file '%s': %s\n", argv[1], sf_strerror(sndFile));
		return 1;
	}

	SF_INFO sndInfoOut = sndInfo;
	sndInfoOut.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	sndInfoOut.channels = 1;
	sndInfoOut.samplerate = sndInfo.samplerate;
	SNDFILE *sndFileOut = sf_open(argv[2], SFM_WRITE, &sndInfoOut);

	// Check format - 16bit PCM
	if (sndInfo.format != (SF_FORMAT_WAV | SF_FORMAT_PCM_16)) {
		fprintf(stderr, "Input should be 16bit Wav\n");
		sf_close(sndFile);
		return 1;
	}

	// Check channels - mono
	if(sndInfo.channels != 1) {
		fprintf(stderr, "Wrong number of channels\n");
		sf_close(sndFile);
		return 1;
	}

	float wc = 2*PI*f/Fs;
	float r = 0.99;
	//Allocate Memory
	float *outputtemp = malloc((sndInfo.frames+2)*sizeof(float));
	float *input = malloc((2+sndInfo.frames)*sizeof(float));
	float *output = malloc(sndInfo.frames*sizeof(float));
	if(output == NULL || input == NULL || outputtemp == NULL){
		fprintf(stderr, "Could not allocate memory for file\n");
		return 1;
	}	

	outputtemp[0] = 0;
	outputtemp[1] = 0;
	
	
	sf_read_float(sndFile, input, sndInfo.frames);

	for(int i = 0; i < sndInfo.frames;i++) {
		input[sndInfo.frames-1-i+2] = input[sndInfo.frames-1-i];
	}

	input[0] = 0;
	input[1] = 0;
	
	for(int i = 2; i < sndInfo.frames+2; i++) {
   	outputtemp[i] = 2*r*cos(wc)*outputtemp[i-1]-r*r*outputtemp[i-2]+input[i]-2*cos(wc)*input[i-1]+input[i-2]; 
	}
	
	output = &outputtemp[2]; 
	

	sf_writef_float(sndFileOut,output, sndInfo.frames);
		
	sf_close(sndFile);
	sf_write_sync(sndFileOut);
	sf_close(sndFileOut);
	free(input);
	free(outputtemp);

	return 1;
}
