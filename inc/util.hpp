#ifndef UTIL_HPP
#define UTIL_HPP
#include <memory>

void print_charstr(const char *msg);
double midi_note_to_freq(unsigned int note);

template <typename T>
std::unique_ptr<T> allocate_unique(void) {
    return std::make_unique<T>();
} 


#endif
