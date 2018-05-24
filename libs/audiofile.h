#ifndef AUDIOFILE_H
#define AUDIOFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>
#include "util.h"

#define AUDIOFILE_TEMP_BUFFER_SIZE 1024*1024
class AudioFile{
  private:
    static float TEMP_BUFFER[AUDIOFILE_TEMP_BUFFER_SIZE];
  public:
    float* buffer;
    int frames;
    int channels;
    int sampleRate;
    AudioFile(float* buffer, int frames, int channels, int sampleRate){
      if(!(this->buffer=(float*)malloc(frames*channels*sizeof(float)))){
        FATAL("Memory allocation failure.");
      };
      memcpy(this->buffer,buffer,frames*channels*sizeof(float));
      this->frames=frames;
      this->channels=channels;
      this->sampleRate=sampleRate;
    }
    AudioFile(const char* path){
      SNDFILE *file;
      SF_INFO sfinfo;
      this->buffer=NULL;
      memset(&sfinfo,0,sizeof(sfinfo));
      if(!(file=sf_open(path,SFM_READ,&sfinfo))){
        this->buffer=NULL;
        ERROR("Could not read file %s.", path);
        return;
      }
      int samplesRead=0;
      int totalSamplesRead=0;
      while((samplesRead=sf_read_float(file,TEMP_BUFFER,AUDIOFILE_TEMP_BUFFER_SIZE))){
        totalSamplesRead+=samplesRead;
        if(this->buffer==NULL){
          this->buffer=(float*)malloc(totalSamplesRead*sizeof(float));
          DEBUG("malloc: %p, %8x, %8x",(void*)this->buffer,totalSamplesRead*sizeof(float),samplesRead*sizeof(float));
        } else {
          this->buffer=(float*)realloc(this->buffer,totalSamplesRead*sizeof(float));
          DEBUG("realloc: %p, %8x, %8x",(void*)this->buffer,totalSamplesRead*sizeof(float),samplesRead*sizeof(float));
        }
        memcpy(this->buffer+(totalSamplesRead-samplesRead),TEMP_BUFFER,samplesRead*sizeof(float));
        DEBUG("memcpy");
      }
      this->sampleRate=sfinfo.samplerate;
      this->channels=sfinfo.channels;
      this->frames=totalSamplesRead/this->channels;
      sf_close(file); 
    }
    ~AudioFile(){if(this->buffer!=NULL)free(this->buffer);}
    void Write(char* path){
      SNDFILE *file;
      SF_INFO sfinfo;
      memset(&sfinfo,0,sizeof(sfinfo));
      sfinfo.samplerate=sampleRate;
      sfinfo.frames=frames;
      sfinfo.channels=channels;
      sfinfo.format=SF_FORMAT_WAV | SF_FORMAT_FLOAT;
      if(!(file=sf_open(path,SFM_WRITE,&sfinfo))){
        ERROR("Couldn't write to file %s.",path);
        return;
      }
      // we have to set frames after opening the file because an empty file sets frames to 0
      sfinfo.frames=frames;
      if(sf_write_float(file,buffer,sfinfo.frames*sfinfo.channels)!=sfinfo.frames*sfinfo.channels){
        ERROR("Failed to write all samples to file %s.",path);
      }
      sf_close(file);
    };
};
float AudioFile::TEMP_BUFFER[AUDIOFILE_TEMP_BUFFER_SIZE];

#endif
