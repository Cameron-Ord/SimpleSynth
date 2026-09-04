#include "../inc/util.hpp"
#include <cmath>

double midi_note_to_freq(unsigned int note){
    return 440.0 * std::exp((std::log(2) / 12) * (static_cast<double>(note) - 69.0));
}