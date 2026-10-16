#include "../inc/synth.hpp"
#include "../inc/util.hpp"
#include <cmath>
#include <iostream>
static constexpr double PI = 3.1415926535897932384626433832795;

static double simple_sine_wave(double phase);
// These functions will have aliasing
static double simple_saw_wave(double phase);
static double simple_square_wave(double phase);
static double simple_triangle_wave(double phase);


static double simple_sine_wave(double phase) {
    return sin(2.0f * PI * phase);
}

static double simple_saw_wave(double phase) {
    return 2.0 * (phase - floor(1.0 / 2.0 + phase));
}

static double simple_square_wave(double phase) {
    return phase < 0.5 ? 1.0 : -1.0;
}

static double simple_triangle_wave(double phase) {
    return 2.0 * fabs(2.0 * phase - floor(phase + (1.0 / 2.0))) - 1.0;
}

Synth::Synth() : osc_config(sample_rate), voices(voice_count, Voice(osc_config)) {}

Voice::Voice(const Oscillator_Configuration& _cfg) : oscs(osc_count, Oscillator_State()), cfg(_cfg) {}

Oscillator_Configuration::Oscillator_Configuration(int sample_rate){
    attack_rate = 1.0 / ms_to_ticks(ATK, sample_rate);
    decay_rate = 1.0 / ms_to_ticks(DEC, sample_rate);
    release_rate = 1.0 / ms_to_ticks(REL, sample_rate);
}

void Oscillator_State::reset(void){
    phase = 0.0;
    time = 0.0;
}

double Oscillator_State::time_increment(double dt){
    return time + dt;
}

double Oscillator_State::phase_increment(double inc){
    return (phase + inc) > Oscillator_State::phase_max ? 0.0 : phase + inc;
}

// https://github.com/mmitch/syndig/blob/master/src/envelope.c#L144
// Used their method for a simple linear adsr configuration
double Voice::env_mult(double sample) {
    return sample * env_value;
}

void Voice::adsr(double dt){
    switch(state) {
        default: break;
        case ATTACKING: {
            if(env_value >= 1.0){
                env_value = 1.0;
                state = DECAYING;
            } else {
                env_value = std::min(env_value + cfg.get_attack_rate(), 1.0);
            }
        }break;

        case DECAYING:{
            if(env_value <= cfg.SUS){
                env_value = cfg.SUS;
                state = SUSTAINING;
            } else {
                env_value = std::max(env_value - cfg.get_decay_rate(), cfg.SUS);
            }
        }break;

        case RELEASING:{
            if(env_value <= 0.0) {
                env_value = 0.0;
                state = OFF;
            } else {
                env_value = std::max(env_value - cfg.get_release_rate(), 0.0);
            }
        }break;
    }
}

void Synth::voice_push_note(double freq, double velocity, unsigned int midi_key) {
    for(auto it = voices.begin(); it != voices.end(); ++it) {
        Voice& v = *it;
        if(v.get_state() == OFF){
            v.set_target_frequency(freq);
            v.set_real_frequency(v.get_prev_frequency() != v.dead_freq ? v.get_prev_frequency() : freq);
            v.set_velocity(velocity);
            for(auto osc_it = v.get_oscs().begin(); osc_it != v.get_oscs().end(); ++osc_it){
                Oscillator_State& osc = *osc_it;
                osc.reset();
            }
            v.reset_env();
            v.set_key(midi_key);
            v.set_state(ATTACKING);
            return;
        }
    }
}

void Synth::voice_send_kill_signal(unsigned int midi_key) {
    for(auto it = voices.begin(); it != voices.end(); ++it) {
        Voice& v = *it;
        const bool busy = v.get_state() != OFF && v.get_state() != RELEASING;
        if(busy && v.get_key() == midi_key) {
            v.set_state(RELEASING);
            v.set_prev_frequency(v.get_target_frequency());
            return;
        }
    }
}

void Synth::run(size_t N, float *hardware_buffer){
    const double dt = 1.0 / sample_rate;
    for(int n = 0; n < N; n++){
        double frame_sample = 0.0;
        
        for(auto it = voices.begin(); it != voices.end(); ++it){
            if(it->get_state() != OFF){
                frame_sample += it->run(dt, *it, sample_rate);
            }
        }

        float compressed = static_cast<float>(tanh(frame_sample * 0.33));
        hardware_buffer[n] = compressed;
    }
}

double Voice::run(double dt, Voice& v, int sample_rate) {
    double voice_sample = 0.0;
    v.adsr(dt);
    std::vector<Oscillator_State>& oscs = v.get_oscs();
    for(auto osc_it = oscs.begin(); osc_it != oscs.end(); ++osc_it){
        Oscillator_State& osc = *osc_it;
        double inc = 0.0;

        const double current = v.get_target_frequency();
        const double prev = v.get_real_frequency();
        const double coeff = glide_coefficient(0.125, sample_rate);
        const double step = v.get_real_frequency() + lerp(current, prev, coeff);
        v.set_real_frequency(step);

        inc = v.get_real_frequency() / sample_rate;
        osc.set_phase(osc.phase_increment(inc));
        voice_sample += simple_saw_wave(osc.get_phase());
    }
    return v.env_mult(voice_sample) * v.get_velocity();
}


