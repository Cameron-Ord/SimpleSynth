#include "../inc/keyboard.hpp"
#include "../inc/util.hpp"
#include "../inc/synth.hpp"

#include <stdexcept>
#include <string>

enum MIDI_CODES {
    //1001nnnn
    NOTE_ON = 0x90,
    NOTE_OFF = 0x80,
};

enum MIDI_ACCESSES {
    STATUS = 0,
    NOTE,
    VELOCITY,
};

// https://midi.org/summary-of-midi-1-0-messages
void midi_callback(double delta_time, std::vector<unsigned char> *message, void *const user_data){
    Synth *const synth = static_cast<Synth *const>(user_data);
    const size_t nbytes = message->size();

    switch((*message)[STATUS]){
        case NOTE_ON : {
            synth->voice_push_note(
                midi_note_to_freq((*message)[NOTE]), 
                midi_velocity_to_float((*message)[VELOCITY]),
                (*message)[NOTE]
            );
        }break;

        case NOTE_OFF : {
            synth->voice_send_kill_signal((*message)[NOTE]);
        }break;
    }
}

void error_callback(RtMidiError::Type type, const std::string &error_text, void *const user_data){
    throw std::runtime_error("RtMidi Error: " + error_text);  
}

MidiContext::MidiContext(void *const error_userdata, void *const synth_userdata) : midi_in(nullptr) {
    initialize();
    print_version();
    set_error_callback(error_callback, nullptr);
    set_callback(midi_callback, synth_userdata);
    open_dft_port();
    midi_in_ignore(false, false, false);
}

void MidiContext::print_version(void) {
    std::cout << "{ RtMidi Version: " << midi_in->getVersion() << " }" << std::endl;
}

void MidiContext::set_error_callback(void (*error_cb)(RtMidiError::Type type, const std::string &error_text, void *const user_data), void *const userdata){
    midi_in->setErrorCallback(error_cb, userdata);
}

void MidiContext::open_dft_port(void){
    midi_in->openPort(0);
}

void MidiContext::initialize(void){
    try {
        midi_in = std::make_unique<RtMidiIn>();
    } catch (RtMidiError &error) {
        throw std::runtime_error("RtMidi Error: " + error.getMessage());  
    } catch (const std::bad_alloc &error) {
        throw;
    } catch (const std::exception &error) {
        throw std::runtime_error("Exception: " + std::string(error.what()));  
    }
}

void MidiContext::set_callback(message_cb callback, void *const userdata){
    midi_in->setCallback(callback, userdata);
}

void MidiContext::midi_in_ignore(bool sysex, bool timing, bool sense){
    midi_in->ignoreTypes(sysex, timing, sense);
}

unsigned int MidiContext::port_count_in() const {
    return midi_in->getPortCount();    
}

std::string MidiContext::get_port_name(unsigned int port_number) const {
    return midi_in->getPortName(port_number);
}

void MidiContext::list_ports_in(unsigned int nports) const {
    for(unsigned int p = 0; p < nports; p++){
        std::string name = get_port_name(p);
        std::cout << name << std::endl;
    }
}

