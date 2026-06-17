#include<stdio.h>
#include<stdlib.h>
#include <windows.h>   // Native Windows Definitions
#include <mmsystem.h>  // Native Windows Multimedia Core

#define DATA_READ_TIME_MSEC 10
typedef struct waveheader
{
 char chunkid[4];
 int chunksize;
 char format[4];
 
 char subchunkid[4];
 int  subchunksize;
 short audioformat;
 short numberofchannel;
 int   samplerate;
 int   byterate;
 short blockalign;
 short bitdepth;
 
 char subchunkid2[4];
 int  subchunksize2;
}metadata;

void printmetadata(struct waveheader head)
{
	printf("\n\n/*****************************************************************/");
	printf("\nchunkid	= %s",head.chunkid);
	printf("\nchunksize	= %d",head.chunksize);
	printf("\nformat 	= %s",head.format);

	printf("\nsubchunkid 	= %s",head.subchunkid);
	printf("\nsubchunksize 	= %d",head.subchunksize);
	printf("\naudioformat	= %d",head.audioformat);
	printf("\nchannels		= %d",head.numberofchannel);
	printf("\nsamplerate	= %d",head.samplerate);
	printf("\nbyterate		= %d",head.byterate);
	printf("\nblockalign 	= %d",head.blockalign);
	printf("\nbitdepth		= %d",head.bitdepth);

	printf("\nsubchunkid2	= %s",head.subchunkid2);
	printf("\nsubchunksize2	= %d",head.subchunksize2);
	
	printf("\n\n/*****************************************************************/");
}

int main(void)
{
  printf("hello...");
     FILE *infile = fopen("sin1000.wav", "rb");
    if(!infile) { printf("\n Error file open"); return 1; }

    metadata header;
    fread(&header, 1, sizeof(metadata), infile);
    printmetadata(header);

    // 1. Setup Windows Audio Format structure using your file metadata
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = header.numberofchannel;
    wfx.nSamplesPerSec = header.samplerate;
    wfx.nAvgBytesPerSec = header.byterate;
    wfx.nBlockAlign = header.blockalign;
    wfx.wBitsPerSample = header.bitdepth;
    wfx.cbSize = 0;

    HWAVEOUT hWaveOut;
    // Open the default windows wave playback device
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        printf("Error opening Windows Audio Device\n");
        return 1;
    }

    // 2. Allocate buffer
    int frames_per_read = (header.samplerate / 1000) * DATA_READ_TIME_MSEC;
    int bytes_per_read = frames_per_read * header.blockalign;
    short *data = (short *) malloc(bytes_per_read);

    // 3. Prepare the tracking header structure Windows forces us to use
    WAVEHDR waveHeader;
    waveHeader.lpData = (LPSTR)data;
    waveHeader.dwBufferLength = bytes_per_read;
    waveHeader.dwFlags = 0;

    printf("\nPlaying via Native Windows waveOut...\n");

    int bytesread;
    while ((bytesread = fread(data, 1, bytes_per_read, infile)) > 0) {
        waveHeader.dwBufferLength = bytesread;

        // Freeze memory page for the audio hardware driver 
        waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));
        
        // Push raw buffer chunk to the speakers asynchronously
        waveOutWrite(hWaveOut, &waveHeader, sizeof(WAVEHDR));

        // Because waveOut is non-blocking, we must loop-wait here until 
        // the driver finishes processing this specific block.
        while (!(waveHeader.dwFlags & WHDR_DONE)) {
            Sleep(1); // Sleep 1ms to prevent 100% CPU usage
        }

        // Unfreeze the memory block
        waveOutUnprepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));
    }

    // 4. Cleanup
    free(data);
    waveOutClose(hWaveOut);
    fclose(infile);
    return 0;  

}

