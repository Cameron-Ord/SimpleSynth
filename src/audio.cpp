#include "../inc/audio.hpp"
#include "../inc/synth.hpp"
#include <iostream>

static constexpr size_t CALLBACK_FRAME_MAX = 128;

static bool stream_feed(SDL_AudioStream *stream, const float samples[], int len);

void stream_get(void *data, SDL_AudioStream *stream, int add, int total) {
  Synth *synth = static_cast<Synth *>(data);
  if (!synth)
    return;
 // Additional is consumed immediately
  (void)total;
  size_t sample_count = (uint32_t)add / sizeof(float);
  while (sample_count > 0) {
    float samples[CALLBACK_FRAME_MAX] = {0};
    const size_t N = SDL_min(sample_count, SDL_arraysize(samples));
    synth->run(/*N, samples*/);
    stream_feed(stream, samples, (int32_t)N * (int32_t)sizeof(float));
    sample_count -= N;
 }
  return;
}

static bool stream_feed(SDL_AudioStream *stream, const float samples[], int len) {
  return SDL_PutAudioStreamData(stream, samples, len);
}

Audio_Sys::Audio_Sys(int chan, int sample_rate)
: dev(0), stream(NULL), internal({SDL_AUDIO_F32, chan, sample_rate}), output({SDL_AUDIO_F32, 0, 0}) {
}

int Audio_Sys::get_num_audio_drivers(void){
    return SDL_GetNumAudioDrivers();
}

void Audio_Sys::list_audio_drivers(int ndrivers){
    for(int i = 0; i < ndrivers; i++){
        const char *name = SDL_GetAudioDriver(i);
        if(name){
            std::cout << name << std::endl;
        }
    }
}

bool Audio_Sys::open(void *userdata) {
  if (!(open_audio_device() && create_audio_stream())) {
    return false;
  }

  if (!(set_audio_callback(userdata) && bind_stream())) {
    return false;
  }
  return true;
}

void Audio_Sys::close(void) {
  pause();
  clear();
  unbind_stream();
  destroy_audio_stream();
  close_audio_device();
}

void Audio_Sys::clear(void) {
  if (!SDL_ClearAudioStream(stream)) {
    std::cerr << "Failed to clear stream: " << SDL_GetError() << std::endl;
  }
}

bool Audio_Sys::set_audio_callback(void *userdata) {
  if (!SDL_SetAudioStreamGetCallback(stream, stream_get, userdata)) {
    std::cerr << "Failed to set callback: " << SDL_GetError() << std::endl;
    return false;
  }
  return true;
}

bool Audio_Sys::bind_stream(void) {
  if (!(stream && dev)) {
    std::cerr << "Invalid parameters" << std::endl;
    return false;
  }

  if (!SDL_BindAudioStream(dev, stream)) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }

  return true;
}

bool Audio_Sys::unbind_stream(void) {
  if (!stream) {
    std::cerr << "Invalid parameters" << std::endl;
    return false;
  }

  SDL_UnbindAudioStream(stream);
  return true;
}

bool Audio_Sys::open_audio_device(void) {
  dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
  if (!dev) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }
  SDL_GetAudioDeviceFormat(dev, &output, NULL);
  const char *name = SDL_GetAudioDeviceName(dev);
  std::cout << "Device name: " << name << std::endl;
  std::cout << "Channels: " << output.channels << " Samplerate: " << output.freq
            << std::endl;
  return true;
}

bool Audio_Sys::create_audio_stream(void) {
  stream = SDL_CreateAudioStream(&internal, &output);
  if (!stream) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }
  return true;
}

bool Audio_Sys::close_audio_device(void) {
  if (!dev) {
    std::cerr << "Invalid parameter" << std::endl;
    return false;
  }
  SDL_CloseAudioDevice(dev);
  return true;
}

bool Audio_Sys::destroy_audio_stream(void) {
  if (!stream) {
    std::cerr << "Invalid parameter" << std::endl;
    return false;
  }
  return true;
}

bool Audio_Sys::resume(void) {
  if (!dev) {
    std::cerr << "Invalid parameter" << std::endl;
    return false;
  }

  if (!SDL_PauseAudioDevice(dev)) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }

  return true;
}

bool Audio_Sys::pause(void) {
  if (!dev) {
    std::cerr << "Invalid parameter" << std::endl;
    return false;
  }

  if (!SDL_ResumeAudioDevice(dev)) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }

  return true;
}