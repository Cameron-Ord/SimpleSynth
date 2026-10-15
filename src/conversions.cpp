#include "../inc/util.hpp"
#include <cmath>
#include <memory>
#include <cstddef>

// https://en.wikipedia.org/wiki/MIDI_tuning_standard
// https://caml.music.mcgill.ca/~gary/307/week1/node11.html

double midi_note_to_freq(unsigned int note){
    return 440.0 * std::exp((std::log(2) / 12) * (static_cast<double>(note) - 69.0));
}
