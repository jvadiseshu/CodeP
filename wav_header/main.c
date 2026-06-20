#include <stdio.h>
#include <stdlib.h>
#include <windows.h>   // Native Windows Definitions
#include <mmsystem.h>  // Native Windows Multimedia Core

// Increased from 10 to 40ms to give the OS driver a safer processing window
#define DATA_READ_TIME_MSEC 40 
#define NUM_BUFFERS 4          // Multi-buffer queue to guarantee zero underruns

#pragma pack(push, 1)
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
} metadata;
#pragma pack(pop)

void printmetadata(metadata head)
{
    printf("\n\n/*****************************************************************/");
    printf("\nchunkid      = %.4s", head.chunkid);
    printf("\nchunksize    = %d", head.chunksize);
    printf("\nformat       = %.4s", head.format);
    printf("\nsubchunkid   = %.4s", head.subchunkid);
    printf("\nsamplerate   = %d Hz", head.samplerate);
    printf("\nchannels     = %d", head.numberofchannel);
    printf("\nbitdepth     = %d bits", head.bitdepth);
    printf("\n/*****************************************************************/\n");
}

int main(void)
{
    FILE *infile = fopen("sin1000.wav", "rb");
    if (!infile) { printf("\n Error file open"); return 1; }

    metadata header;
    if (fread(&header, 1, sizeof(metadata), infile) != sizeof(metadata)) {
        printf("Error reading metadata\n");
        fclose(infile);
        return 1;
    }
    printmetadata(header);

    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = header.numberofchannel;
    wfx.nSamplesPerSec = header.samplerate;
    wfx.nAvgBytesPerSec = header.byterate;
    wfx.nBlockAlign = header.blockalign;
    wfx.wBitsPerSample = header.bitdepth;
    wfx.cbSize = 0;

    HWAVEOUT hWaveOut;
    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        printf("Error opening Windows Audio Device\n");
        fclose(infile);
        return 1;
    }

    int frames_per_read = (header.samplerate / 1000) * DATA_READ_TIME_MSEC;
    int bytes_per_read = frames_per_read * header.blockalign;
    
    // Allocate resources array dynamically based on buffer depth count
    WAVEHDR waveHeader[NUM_BUFFERS];
    short *buffer_data[NUM_BUFFERS];

    for (int i = 0; i < NUM_BUFFERS; i++) {
        buffer_data[i] = (short *)malloc(bytes_per_read);
        memset(&waveHeader[i], 0, sizeof(WAVEHDR));
        waveHeader[i].lpData = (LPSTR)buffer_data[i];
        waveHeader[i].dwBufferLength = bytes_per_read;
    }

    printf("Playing gapless audio via Ring-Queued Windows waveOut...\n");

    int current_buffer = 0; 
    int bytesread;
    int active_buffers = 0;

    // 1. PRIME PHASE: Completely fill the queue immediately
    // The driver will process these in order without gaps.
    for (int i = 0; i < NUM_BUFFERS; i++) {
        bytesread = fread(waveHeader[i].lpData, 1, bytes_per_read, infile);
        if (bytesread > 0) {
            waveHeader[i].dwBufferLength = bytesread;
            waveOutPrepareHeader(hWaveOut, &waveHeader[i], sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &waveHeader[i], sizeof(WAVEHDR));
            active_buffers++;
        }
    }

    // 2. RUNNING STREAM PHASE
    while (active_buffers > 0) {
        WAVEHDR *hdr = &waveHeader[current_buffer];

        // Wait until the hardware finishes playing this specific index
        while (!(hdr->dwFlags & WHDR_DONE)) {
            Sleep(1); 
        }

        // Clean up the spent header structure safely
        waveOutUnprepareHeader(hWaveOut, hdr, sizeof(WAVEHDR));
        active_buffers--;

        // Instantly read next file block to overwrite the spent buffer index space
        bytesread = fread(hdr->lpData, 1, bytes_per_read, infile);
        if (bytesread > 0) {
            hdr->dwBufferLength = bytesread;
            waveOutPrepareHeader(hWaveOut, hdr, sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, hdr, sizeof(WAVEHDR)); // Queue it to back of driver line
            active_buffers++;
        }

        // Shift target index to trace the next upcoming expected finished buffer block
        current_buffer = (current_buffer + 1) % NUM_BUFFERS;
        
        // If file ended, cleanly transition to trailing wind-down loop logic
        if (bytesread <= 0) {
            break; 
        }
    }

    // 3. WIND-DOWN TRAILING BUFFER PHASE
    // Wait for the remaining items inside the hardware queue to finish rendering cleanly
    while (active_buffers > 0) {
        WAVEHDR *hdr = &waveHeader[current_buffer];
        while (!(hdr->dwFlags & WHDR_DONE)) {
            Sleep(1);
        }
        waveOutUnprepareHeader(hWaveOut, hdr, sizeof(WAVEHDR));
        active_buffers--;
        current_buffer = (current_buffer + 1) % NUM_BUFFERS;
    }

    // 4. CLEANUP RESOURCE ARRAYS
    for (int i = 0; i < NUM_BUFFERS; i++) {
        free(buffer_data[i]);
    }
    waveOutClose(hWaveOut);
    fclose(infile);
    
    printf("Playback Finished Gaplessly.\n");
    return 0;  
}
