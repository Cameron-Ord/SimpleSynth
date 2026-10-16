#ifndef AUDIO_SYS_HPP
#define AUDIO_SYS_HPP
#include "SDL3/SDL_audio.h"

void stream_get(void *data, SDL_AudioStream *stream, int add, int total);

class Audio_Sys {
public:
  Audio_Sys(int synth_channels, int synth_sample_rate);
  ~Audio_Sys(void) = default;
  int get_num_audio_drivers(void);
  void list_audio_drivers(int ndrivers);
  void open(void *const userdata);
  void close(void);
  void set_audio_callback(void *const userdata);
  void bind_stream(void);
  void unbind_stream(void) noexcept;
  void open_audio_device(void);
  void create_audio_stream(void);
  void close_audio_device(void) noexcept;
  void destroy_audio_stream(void) noexcept;
  bool resume(void) const noexcept;
  bool pause(void) const noexcept;
  void clear(void) noexcept;

private:
  unsigned int dev;
  SDL_AudioStream *stream;
  SDL_AudioSpec internal;
  SDL_AudioSpec output;
};

#endif