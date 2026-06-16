#include<stdio.h>
#include<stdlib.h>
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
  FILE *infile = fopen("1kHz_44100Hz_16bit_30sec.wav","r+");
  
  if(infile == NULL)
	printf("\n Error in file open");
  else
	printf("\n File open sucess return = %p", infile);

  metadata header;
  int bytesread = fread(&header,1, sizeof(metadata),infile);

  printf("\n %s: bytes read from input file = %d", __func__, bytesread);
  printmetadata(header);
  
  short *data = (short *) malloc(header.blockalign * (header.samplerate/1000) * DATA_READ_TIME_MSEC );
  
  //get a block align data
  for(int i=0; i<10;i++)
  {
   bytesread = fread(data, 1, header.blockalign, infile);
   printf("\ndata read in bytes = %d, data = %x",bytesread, *data);
   data++;
  }

    
 fclose(infile);
  
  return 0;
}

