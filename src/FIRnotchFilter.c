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

	//Allocate Memory
	float *notchfilteredfir = malloc((sndInfo.frames+14)*sizeof(float));
	float *ycorr = malloc(14+14+sndInfo.frames*sizeof(float));
	if(ycorr == NULL || notchfilteredfir == NULL){
		fprintf(stderr, "Could not allocate memory for file\n");
		return 1;
	}


float filtercoeff[15] = {0};
float wc = 2*PI*f/Fs;
for(int i = 0; i < 15; i++){
    filtercoeff[i] = 2*sin(wc*(i-7))/(PI*(i-7));
}
filtercoeff[7] = 2*(wc/PI-0.5);

sf_read_float(sndFile, ycorr, sndInfo.frames);
	
	for(int i = 0; i < sndInfo.frames;i++) {
		ycorr[sndInfo.frames-1-i+14] = ycorr[sndInfo.frames-1-i];
	}

	for(int i = 0; i < 14; i++) {
		ycorr[i] = 0;
	}

	for(int i = 0; i < 14; i++) {
		ycorr[14+sndInfo.frames+i] = 0;
	}

for(int i = 0; i < sndInfo.frames+14; i++) {
   notchfilteredfir[i] = 0;
	for(int k = 0; k < 15; k++) {
		notchfilteredfir[i] = notchfilteredfir[i] + filtercoeff[14-k]*ycorr[14+i-k];
	}
}	


	sf_writef_float(sndFileOut,notchfilteredfir, sndInfo.frames+14);
		
	sf_close(sndFile);
	sf_write_sync(sndFileOut);
	sf_close(sndFileOut);
	free(notchfilteredfir);
	free(ycorr);

	return 1;
}
