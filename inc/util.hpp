#ifndef UTIL_HPP
#define UTIL_HPP
#include <memory>

void print_charstr(const char *msg);
double midi_note_to_freq(unsigned int note);
double midi_velocity_to_float(unsigned int velocity);
double ms_to_ticks(unsigned int ms, int sample_rate);

template <typename T>
std::unique_ptr<T> allocate_unique(void) {
    return std::make_unique<T>();
} 


#endif
