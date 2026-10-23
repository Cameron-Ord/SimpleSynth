#ifndef UTIL_HPP
#define UTIL_HPP
#include <memory>

void print_charstr(const char *msg);
double midi_note_to_freq(unsigned int note);
double normalized_midi_velocity(unsigned int velocity);
double ms_to_ticks(unsigned int ms, int sample_rate);
int nyquist_frequency(int sample_rate);
double step(double current, double prev, double coefficient);
double clamp_exponential(double EPS, double target, double actual);
double ms_to_seconds(unsigned int ms);
unsigned int seconds_to_ms(double s);
double squared_log10(double normalized);
double undo_log_10(double log);
double normalize_decibels(double DB, double factor = 20.0, double db_min = -60.0, double db_max = 0.0);
double normalized_amp_to_normalized_decibels(unsigned int velocity, double factor = 20.0);

template <typename T>
std::unique_ptr<T> allocate_unique(void) {
    return std::make_unique<T>();
} 


#endif
