#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP
#include <vector>
#include <string>
#include <memory>

#include <RtMidi.h>
void midi_callback(double delta_time, std::vector<unsigned char> *message, void *const user_data);
void error_callback(RtMidiError::Type type, const std::string &error_text, void* user_data);

typedef void (*error_cb)(RtMidiError::Type, const std::string &, void *const);
typedef void (*message_cb)(double, std::vector<unsigned char>*, void *const);

class MidiContext {
public:
    MidiContext(void *const error_userdata, void *const synth_userdata);
    ~MidiContext() = default;
    void set_error_callback(error_cb callback, void *const userdata);
    void print_version(void);
    void open_dft_port(void);
    void initialize(void);
    void set_callback(message_cb callback, void *const userdata);
    std::string get_port_name(unsigned int port_number) const;
    unsigned int port_count_in(void) const;
    void list_ports_in(unsigned int nports) const;
    void midi_in_ignore(bool sysex, bool timing, bool sense);
private:
    std::unique_ptr<RtMidiIn> midi_in;
};



#endif
