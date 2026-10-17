#include "../inc/synth.hpp"
#include "../inc/util.hpp"
#include <cmath>
#include <iostream>
static constexpr double PI = 3.1415926535897932384626433832795;

static double naive_sine_wave(double phase);
// These functions will have aliasing
static double naive_saw_wave(double phase);
static double naive_square_wave(double phase, double cycle);
static double naive_triangle_wave(double phase);

static double poly_blep(double t, double dt);
static void compressor(size_t N, float *buffer);
static double synth_tree(size_t type, double phase, double phase_inc, double pulse_cycle);

//https://www.martin-finke.de/articles/audio-plugins-018-polyblep-oscillator/
//https://www.metafunction.co.uk/post/all-about-digital-oscillators-part-2-blits-bleps
static double poly_blep(double t, double dt) {
    if(t < dt) {
        t /= dt;
        return t+t - t*t - 1.0;
    } else if (t > 1.0 - dt){
        t = (t - 1.0) / dt;
        return t*t + t+t + 1.0;
    } else {
        return 0.0;
    }
} 

static double synth_tree(size_t type, double phase, double phase_inc, double pulse_cycle) {
    // Don't need to normalize by 2.0 * PI since things are already 0..1
    const double t = phase;
    const double dt = phase_inc;
    switch(type) {
        default: return 0.0;
        case SINE : {
            return naive_sine_wave(phase);
        }break;
        case SAW : {
            return naive_saw_wave(phase) - poly_blep(t, dt);
        }break;
        case SQUARE : {
            return naive_square_wave(phase, 0.5) + poly_blep(t, dt) - poly_blep(fmod(t + 0.5, 1.0), dt);
        }break;
        case TRIANGLE : {
            return naive_triangle_wave(phase);
        }break;
        case PULSE: {
            return naive_square_wave(phase, pulse_cycle) + poly_blep(t, dt) - poly_blep(fmod(t + (1.0 - pulse_cycle), 1.0), dt);
        }break;
    }
}

constexpr double neutral = 1.0;
constexpr double falloff = 0.01;

//https://github.com/mmitch/syndig/blob/master/src/compressor.c
static void compressor(size_t N, float *buffer){
    static bool compressing = false;
    static double compression = neutral;
    double min = 0.0;
    double max = 0.0;
    for(size_t n = 0; n < N; n++){
        if(buffer[n] < min) {
            min = buffer[n];
        } else if (buffer[n] > max){
            max = buffer[n];
        }
    }
    max = std::max(max, -min);
    if(max > compression){
        compression = max;
        if(!compressing){
            compressing = true;
        }
    } else if(compression > neutral) {
        compression = std::max(compression - falloff, neutral);
        if(compression == neutral){
            compressing = false;
        }
    }
    if(compressing){
        for(size_t n = 0; n < N; n++){
            buffer[n] /= compression;
        }
    }
}

//https://en.wikipedia.org/wiki/Low-pass_filter#Simple_infinite_impulse_response_filter
// RC = 1 / 2*PI*Freq
// dt = 1 / sample_rate
// a := dt / (RC + dt)
static double low_pass_filter(double dt, double cutoff_hz, double current, double prev) {
    const double RC = 1.0 / (2.0 * PI * cutoff_hz);
    const double alpha = dt / (RC + dt);

    return prev + lerp(current, prev, alpha);
}

static double naive_sine_wave(double phase) {
    return sin(2.0f * PI * phase);
}

static double naive_saw_wave(double phase) {
    return 2.0 * (phase - floor(1.0 / 2.0 + phase));
}

static double naive_square_wave(double phase, double cycle) {
    return phase < cycle ? 1.0 : -1.0;
}

static double naive_triangle_wave(double phase) {
    return 2.0 * fabs(2.0 * phase - floor(phase + (1.0 / 2.0))) - 1.0;
}

double Lfo::sinusoid(){
    return sin(2.0f * PI * phase) * depth;
}

double Lfo::increment(int sample_rate){
    return (phase + (rate / sample_rate)) >= phase_max ? 0.0 : phase + (rate / sample_rate);
}

Synth::Synth() : osc_config(sample_rate, { SAW }), voices(voice_count, Voice(osc_config)) {}

Voice::Voice(const Oscillator_Configuration& _cfg) : cfg(_cfg), oscs(_cfg.get_osc_count(), Oscillator_State()){}

Oscillator_Configuration::Oscillator_Configuration(int sample_rate, std::vector<size_t> _types) : types(_types) {
    attack_rate = 1.0 / ms_to_ticks(ATK, sample_rate);
    decay_rate = 1.0 / ms_to_ticks(DEC, sample_rate);
    release_rate = 1.0 / ms_to_ticks(REL, sample_rate);
}

size_t Oscillator_Configuration::get_type_at(size_t i) const {
    if(i < types.size()){
        return types[i];
    }
    return SAW;
}


void Oscillator_State::reset(void){
    phase = 0.0;
    time = 0.0;
}

double Oscillator_State::time_increment(double dt){
    return time + dt;
}

double Oscillator_State::phase_increment(double inc){
    return (phase + inc) >= phase_max ? 0.0 : phase + inc;
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

void Synth::run(size_t N, float *samples){
    const double dt = 1.0 / sample_rate;
    for(int n = 0; n < N; n++){
        double frame_sample = 0.0;
        for(auto it = voices.begin(); it != voices.end(); ++it){
            if(it->get_state() != OFF){
                frame_sample += it->run(dt, sample_rate);
            } else {
                continue;
            }
        }
        samples[n] = static_cast<float>(frame_sample);
    }
    compressor(N, samples);
}

double Voice::run(double dt, int sample_rate) {
    double voice_sample = 0.0;
    adsr(dt);
    const double coeff = glide_coefficient(0.1, sample_rate);
    real_freq = real_freq + lerp(target_freq, real_freq, coeff);
    for(size_t i = 0; i < oscs.size() && i < cfg.get_osc_count(); i++){
        Oscillator_State& osc = oscs[i];
        const double inc = real_freq / sample_rate;
        osc.set_phase(osc.phase_increment(inc));
        voice_sample += synth_tree(cfg.get_type_at(i), osc.get_phase(), inc, cfg.pulse_duty_cycle);
    }
    filter_prev = low_pass_filter(dt, 2000.0, voice_sample, filter_prev);
    return env_mult(filter_prev) * velocity;
}


