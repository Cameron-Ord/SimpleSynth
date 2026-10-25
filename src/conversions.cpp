#include "../inc/util.hpp"
#include <cmath>
#include <memory>
#include <cstddef>
#include <iostream>

static constexpr unsigned int midi_velocity_max = 127;

// https://en.wikipedia.org/wiki/MIDI_tuning_standard
// https://caml.music.mcgill.ca/~gary/307/week1/node11.html

double midi_note_to_freq(unsigned int note){
    return 440.0 * std::exp((std::log(2) / 12) * (static_cast<double>(note) - 69.0 /*funny number*/));
}

//https://ccrma.stanford.edu/~jos//thms/Decibels.html
//Using log10 and squared normalized ampitude value
double squared_log10(double normalized) {
    return log10(normalized * normalized);
}

double undo_log_10(double log) {
    const double base = 10.0;
    return std::pow(base, log);
}
// value - min / max - min
double normalize_decibels(double db, double factor, double db_min, double db_max){
    return (db - db_min) / (db_max - db_min);
}

double normalized_amp_to_normalized_decibels(unsigned int velocity, double factor) {
    const double decibels = factor * std::log10(normalized_midi_velocity(velocity));
    return normalize_decibels(decibels, factor);
}

double normalized_midi_velocity(unsigned int velocity){
    return static_cast<double>(velocity) / midi_velocity_max;
}

double ms_to_ticks(unsigned int ms, int sample_rate) {
    return static_cast<double>(ms) * sample_rate / 1000;
}

double ms_to_seconds(unsigned int ms) { 
    return ms / 1000.0;
}

unsigned int seconds_to_ms(double s){
    return s * 1000.0;
}

int nyquist_frequency(int sample_rate) {
    return sample_rate / 2;
}

double step(double current, double prev, double coefficient) {
    return prev + (current - prev) * coefficient;
}

double clamp_exponential(double EPS, double target, double actual){
    return std::abs(target - actual) <= EPS ? target : actual;   
}
