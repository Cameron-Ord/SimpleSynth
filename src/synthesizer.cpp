#include "../inc/synth.hpp"
#include "../inc/util.hpp"
#include <cmath>
#include <iostream>
static constexpr double PI = 3.1415926535897932384626433832795;
static constexpr double EPS = 0.01;

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

    return step(current, prev, alpha);
}
// https://www.musicdsp.org/en/latest/Filters/142-state-variable-filter-chamberlin-version.html
// https://www.earlevel.com/main/2003/03/02/the-digital-state-variable-filter/
// f = 2 * sin((PI * Fc) / SAMPLE_RATE) or f = 2 * sin(PI * Fc * dt) since dt = 1.0 / sample_rate
// where Fc is corner/cutoff in Hz
// Q(resonance) ranges from 0.5 to inf
double Voice::state_variable_filter(double input, double cutoff, double dt, double Q){
    const double tuning = 2.0 * sin(PI * cutoff * dt);
    const double q = 1.0 / Q;

    const double low = filter[FILTER_SVF_DELAY_LOWPASS] + tuning * filter[FILTER_SVF_DELAY_BANDPASS];
    const double high = input - low - q * filter[FILTER_SVF_DELAY_BANDPASS];
    const double band = tuning * high + filter[FILTER_SVF_DELAY_BANDPASS];
    const double notch = high + low;

    filter[FILTER_SVF_DELAY_BANDPASS] = band;
    filter[FILTER_SVF_DELAY_LOWPASS] = low;

    return low;
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

double Lfo::increment(double inc){
    return (phase + inc) >= phase_max ? 0.0 : phase + inc;
}

double Lfo::get_inc(int sample_rate) {
    return rate / sample_rate;
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
            if(env_value <= cfg.get_sustain()){
                env_value = cfg.get_sustain();
                state = SUSTAINING;
            } else {
                env_value = std::max(env_value - cfg.get_decay_rate(), cfg.get_sustain());
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

void Voice::reset_filters(){
    for(size_t i = 0; i < filter.size(); i++){
        filter[i] = 0.0;
    }
}

bool Voice::new_note_reset(double _freq, double _velocity, unsigned int _midi_key){
    if(state == OFF){
        set_target_frequency(_freq);
        set_real_frequency(get_prev_frequency() != dead_freq ? get_prev_frequency() : _freq);
        set_velocity(_velocity);
        for(auto it = get_oscs().begin(); it != get_oscs().end(); ++it){
            it->reset();
        }
        reset_env();
        reset_filters();
        set_key(_midi_key);
        set_state(ATTACKING);
        return true;
    } else {
        return false;
    }
}

void Synth::voice_push_note(double freq, double velocity, unsigned int midi_key) {
    for(auto it = voices.begin(); it != voices.end(); ++it) {
        if(it->new_note_reset(freq, velocity, midi_key)){
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
    for(int n = 0; n < N; n++){
        double frame_sample = 0.0;
        for(auto it = voices.begin(); it != voices.end(); ++it){
            if(it->get_state() != OFF){
                frame_sample += it->run(sample_rate);
            } else {
                continue;
            }
        }
        samples[n] = static_cast<float>(frame_sample);
    }
    compressor(N, samples);
}

double Voice::run_low_pass(double dt, double voice_sample){
    const double GAIN = cfg.get_gain();
    switch(cfg.get_distortion_state()){
        default: return 0.0;
        case DISTORTION_ENABLED:{
            filter[FILTER_PRE_DIST] = low_pass_filter(dt, 16000.0, voice_sample, filter[FILTER_PRE_DIST]);
            filter[FILTER_POST_DIST] = low_pass_filter(dt, 16000.0, tanh(filter[FILTER_PRE_DIST] * GAIN), filter[FILTER_POST_DIST]);
            return filter[FILTER_POST_DIST];
        }break;

        case DISTORTION_DISABLED:{
            filter[FILTER_PRE_DIST] = low_pass_filter(dt, 16000.0, voice_sample, filter[FILTER_PRE_DIST]);
            return filter[FILTER_PRE_DIST];
        }break;
    }
}

double Voice::run(int sample_rate) {
    const int over_sampling = 2;
    const double over_sampled_rate = sample_rate * over_sampling;
    const double dt = 1.0 / over_sampled_rate;
    //Things done in normal time
    adsr(dt);
    const double base_cutoff_hz = 500.0;
    const double max_cutoff_hz = 3500.0;
    const double m_cutoff = base_cutoff_hz + (env_value * max_cutoff_hz);
    // Portamento freq stepping
    const double coeff = 1.0 / ms_to_ticks(portamento_time_ms, sample_rate);
    real_freq = clamp_exponential(EPS, target_freq, step(target_freq, real_freq, coeff));

    double accumulator = 0.0;
    for(int c = 0; c < over_sampling; c++){
        double voice_sample = 0.0;
        for(size_t i = 0; i < oscs.size() && i < cfg.get_osc_count(); i++){
            voice_sample += oscs[i].run(cfg, i, real_freq, over_sampled_rate);
        }

        const double lowpass_sample = run_low_pass(dt, voice_sample);
        const double state_variable_out = state_variable_filter(lowpass_sample, m_cutoff, dt, 0.707);
        accumulator += state_variable_out;
        // accumulator += lowpass_sample;
    }
    return env_mult(accumulator / over_sampling) * velocity;
}

double Oscillator_State::run(const Oscillator_Configuration& cfg, size_t i, double freq, double sample_rate) {
    const double phase_inc = freq / sample_rate;
    set_phase(phase_increment(phase_inc));
    return synth_tree(cfg.get_type_at(i), get_phase(), phase_inc, cfg.get_cycle());
}

