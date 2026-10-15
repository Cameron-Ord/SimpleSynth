#include "../inc/keyboard.hpp"
#include "../inc/util.hpp"
#include "../inc/synth.hpp"

#include <stdexcept>
#include <string>

// https://midi.org/summary-of-midi-1-0-messages
// Need to inject a synth class pointer to this in order to do anything
void midi_callback(double delta_time, std::vector<unsigned char> *message, void *const user_data){
    Synth *const synth = static_cast<Synth *const>(user_data);
    unsigned int nbytes = message->size();
    for(unsigned int i = 0; i < nbytes; i++){
        std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
        if ( nbytes > 0 )
            std::cout << "stamp = " << delta_time << std::endl;
    }
}

void error_callback(RtMidiError::Type type, const std::string &error_text, void *const user_data){
    throw std::runtime_error("RtMidi Error: " + error_text);  
}

MidiContext::MidiContext() : midi_in(nullptr) {
    initialize();
    print_version();
    set_error_callback(error_callback);
    set_callback(midi_callback);
    open_dft_port();
    midi_in_ignore(false, false, false);
}

MidiContext::~MidiContext(){
    
}

void MidiContext::print_version(void) {
    std::cout << "{ RtMidi Version: " << midi_in->getVersion() << " }" << std::endl;
}

void MidiContext::set_error_callback(void (*error_cb)(RtMidiError::Type type, const std::string &error_text, void *const user_data)){
    midi_in->setErrorCallback(error_cb);
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

void MidiContext::set_callback(void (*cb)(double, std::vector<unsigned char> *, void *)){
    midi_in->setCallback(cb);
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

