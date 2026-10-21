#include "../inc/util.hpp"
#include <cmath>
#include <memory>
#include <cstddef>

static constexpr unsigned int midi_range_max = 127;

// https://en.wikipedia.org/wiki/MIDI_tuning_standard
// https://caml.music.mcgill.ca/~gary/307/week1/node11.html

double midi_note_to_freq(unsigned int note){
    return 440.0 * std::exp((std::log(2) / 12) * (static_cast<double>(note) - 69.0));
}

double midi_velocity_to_float(unsigned int velocity){
    return static_cast<double>(velocity) / midi_range_max;
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
