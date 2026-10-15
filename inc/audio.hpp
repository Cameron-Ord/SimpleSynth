#ifndef AUDIO_SYS_HPP
#define AUDIO_SYS_HPP
#include "SDL3/SDL_audio.h"

void stream_get(void *data, SDL_AudioStream *stream, int add, int total);

class Audio_Sys {
public:
  Audio_Sys(int chan, int sample_rate);
  ~Audio_Sys(void) = default;
  int get_num_audio_drivers(void);
  void list_audio_drivers(int ndrivers);
  bool open(void *const userdata);
  void close(void);
  bool set_audio_callback(void *const userdata);
  bool bind_stream(void);
  bool unbind_stream(void);
  bool open_audio_device(void);
  bool create_audio_stream(void);
  bool close_audio_device(void);
  bool destroy_audio_stream(void);
  bool resume(void);
  bool pause(void);
  void clear(void);

private:
  unsigned int dev;
  SDL_AudioStream *stream;
  SDL_AudioSpec internal;
  SDL_AudioSpec output;
};

#endif