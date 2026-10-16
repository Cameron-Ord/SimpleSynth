#include "../inc/audio.hpp"
#include "../inc/synth.hpp"
#include <iostream>

static constexpr size_t CALLBACK_FRAME_MAX = 128;
static constexpr float zeroed[CALLBACK_FRAME_MAX]{};

static bool stream_feed(SDL_AudioStream *stream, const float samples[], int len);

void stream_get(void *const user_data, SDL_AudioStream *stream, int add, int total) {
  Synth *const synth = static_cast<Synth *const>(user_data);
  if (!synth)
    return;
 // Additional is consumed immediately
  (void)total;
  size_t sample_count = (uint32_t)add / sizeof(float);
  while (sample_count > 0) {
    float samples[CALLBACK_FRAME_MAX];
    memcpy(samples, zeroed, sizeof(float)*CALLBACK_FRAME_MAX);
    const size_t N = SDL_min(sample_count, SDL_arraysize(samples));
    synth->run(N, samples);
    stream_feed(stream, samples, (int32_t)N * (int32_t)sizeof(float));
    sample_count -= N;
 }
  return;
}

static bool stream_feed(SDL_AudioStream *stream, const float samples[], int len) {
  return SDL_PutAudioStreamData(stream, samples, len);
}

Audio_Sys::Audio_Sys(int synth_channels, int synth_sample_rate)
: dev(0), stream(NULL), internal({SDL_AUDIO_F32, synth_channels, synth_sample_rate}), output({SDL_AUDIO_F32, 0, 0}) {
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

void Audio_Sys::open(void *const userdata) {
  open_audio_device();
  create_audio_stream();
  set_audio_callback(userdata);
  bind_stream();
}

void Audio_Sys::close(void) {
  pause();
  clear();
  unbind_stream();
  destroy_audio_stream();
  close_audio_device();
}

void Audio_Sys::clear(void) noexcept {
  if (!SDL_ClearAudioStream(stream)) {
    std::cerr << "Failed to clear stream: " << SDL_GetError() << std::endl;
  }
}

void Audio_Sys::set_audio_callback(void *const userdata) {
  if (!SDL_SetAudioStreamGetCallback(stream, stream_get, userdata)) {
    throw std::runtime_error(std::string(SDL_GetError()));
  }
}

void Audio_Sys::bind_stream(void) {
  if (!SDL_BindAudioStream(dev, stream)) {
    throw std::runtime_error(std::string(SDL_GetError()));
  }
}

void Audio_Sys::unbind_stream(void) noexcept {
  SDL_UnbindAudioStream(stream);
}

void Audio_Sys::open_audio_device(void) {
  dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
  if (!dev) {
    throw std::runtime_error(std::string(SDL_GetError()));
  }

  SDL_GetAudioDeviceFormat(dev, &output, NULL);
  const char *name = SDL_GetAudioDeviceName(dev);
  if(name){
    std::cout << "Output Device Name: " << name << std::endl 
    << "Output Device Channels: " << output.channels << std::endl 
    << "Output Device Sample Rate: " << output.freq << std::endl;
  }
}

void Audio_Sys::create_audio_stream(void) {
  stream = SDL_CreateAudioStream(&internal, &output);
  if (!stream) {
    throw std::runtime_error(std::string(SDL_GetError()));
  }
}

void Audio_Sys::close_audio_device(void) noexcept {
  if (!dev) {
    return;
  }
  SDL_CloseAudioDevice(dev);
}

void Audio_Sys::destroy_audio_stream(void) noexcept {
  if (!stream) {
    return;
  }
  SDL_DestroyAudioStream(stream);
}

bool Audio_Sys::resume(void) const noexcept {
  if (!SDL_PauseAudioDevice(dev)) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }

  return true;
}

bool Audio_Sys::pause(void) const noexcept {
  if (!SDL_ResumeAudioDevice(dev)) {
    std::cerr << SDL_GetError() << std::endl;
    return false;
  }

  return true;
}