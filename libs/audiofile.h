#ifndef AUDIOFILE_H
#define AUDIOFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

class AudioFile{
  private:
  public:
    float* buffer;
    int frames;
    int channels;
    int sampleRate;
    AudioFile(float* buffer, int frames, int channels, int sampleRate){
      if(!(this->buffer=(float*)malloc(frames*channels*sizeof(float)))){
        printf("MALLOC ERROR!\n");
        exit(1);
      };
      memcpy(this->buffer,buffer,frames*channels*sizeof(float));
      this->frames=frames;
      this->channels=channels;
      this->sampleRate=sampleRate;
    }
    void Write(char* path){
      SNDFILE *file;
      SF_INFO sfinfo;
      memset(&sfinfo,0,sizeof(sfinfo));
      sfinfo.samplerate=sampleRate;
      sfinfo.frames=frames;
      sfinfo.channels=channels;
      sfinfo.format=SF_FORMAT_WAV | SF_FORMAT_FLOAT;
      if(!(file=sf_open(path,SFM_WRITE,&sfinfo))){
        printf("Couldn't write to file!\n");
      }
      // we have to set frames after opening the file because an empty file sets frames to 0
      sfinfo.frames=frames;
      if(sf_write_float(file,buffer,sfinfo.channels*sfinfo.frames)!=sfinfo.channels*sfinfo.frames){
        printf("Failed to write all samples to disk.\n");
      }
      sf_close(file);
    };
};

#endif
