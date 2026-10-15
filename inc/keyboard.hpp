#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP
#include <vector>
#include <string>
#include <memory>

#include <RtMidi.h>
void midi_callback(double delta_time, std::vector<unsigned char> *message, void *user_data);
void error_callback(RtMidiError::Type type, const std::string &error_text, void* user_data);

class MidiContext {
public:
    MidiContext();
    ~MidiContext();
    void set_error_callback(void (*error_cb)(RtMidiError::Type type, const std::string &errorText, void *userData));
    void print_version(void);
    void open_dft_port(void);
    void initialize(void);
    void set_callback(void (*func)(double, std::vector<unsigned char>*, void*));
    std::string get_port_name(unsigned int port_number) const;
    unsigned int port_count_in(void) const;
    void list_ports_in(unsigned int nports) const;
    void midi_in_ignore(bool sysex, bool timing, bool sense);
private:
    std::unique_ptr<RtMidiIn> midi_in;
};



#endif
