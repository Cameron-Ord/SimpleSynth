#include "../inc/synth.hpp"

static double simple_sine_wave(void);
// These functions will have aliasing
static double simple_saw_wave(void);
static double simple_square_wave(void);
static double simple_triangle_wave(void);
static double simple_pulse_wave(void);


Oscillator_State::Oscillator_State() {

}

void Oscillator_State::reset(void){
    phase = 0.0;
    time = 0.0;
}

double Oscillator_State::time_increment(double dt){
    return time + dt;
}

double Oscillator_State::phase_increment(double inc){
    return (phase + inc) > Oscillator_State::phase_max ? 1.0 : phase + inc;
}

Oscillator_Configuration::Oscillator_Configuration(){

}

Synth::Synth(){

}

Voice::Voice(){

}

void Synth::run(void){
    return;
}
