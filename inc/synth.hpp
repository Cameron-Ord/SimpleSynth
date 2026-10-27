#ifndef SYNTH_HPP
#define SYNTH_HPP
#include <vector>
#include <array>

enum Wave_Indexes : size_t{
    SINE = 0,
    SAW,
    SQUARE,
    TRIANGLE,
    PULSE,
};

enum Envelope_States : unsigned int {
    ATTACKING = 0, 
    SUSTAINING,
    DECAYING,
    RELEASING,
    OFF,
};

enum Indexes {
    FILTER_ONE_POLE = 0,
    FILTER_SVF_DELAY_BANDPASS,
    FILTER_SVF_DELAY_LOWPASS,
    INDEXES_SIZE,

    LFO_VIBRATO = 0,
    LFO_SIZE
};

enum Settings {
    DISTORTION_ENABLED = 0,
    USE_SOFT_CLIPPING,
};

class Voice;
class Oscillator_State;
class Synth_Parameters;
class Synth;
class Lfo;

class Compressor {
public:
    Compressor(void) = default;
    void block_compressor(size_t N, float *buffer);
    void clip_buffer(size_t N, float *buffer);
    bool get_state(void) const { return compressing; }
    unsigned int get_flags(void) const { return flags; }
    double get_compressor_gain(void) const { return gain; }
    double get_compressor_attack(void) const { return attack; }
    double get_compressor_release(void) const { return release; }
private:
    unsigned int flags = DISTORTION_ENABLED | USE_SOFT_CLIPPING;
    bool compressing = false;
    double gain = 2.5;
    double attack = 0.05;
    double release = 0.01;
};

class Lfo {
public:
    static constexpr double phase_max = 1.0;
    Lfo() = default;
    ~Lfo() = default;
    double sinusoid(void);
    double get_inc(int sample_rate);
    double increment(double inc);
    void set_phase(double p) { phase = p; }
    double get_phase(void) { return phase; }
    double run(int sample_rate);
private:
    double phase = 0.0;
    double rate = 6.5;
    double depth = 2.0;
};


class Oscillator {
    public:
        static constexpr double DETUNE_MAX = 1.1;
        static constexpr double DETUNE_MIN = 0.9;
        static constexpr double AMPLITUDE_MAX = 1.0;
        static constexpr double AMPLITUDE_MIN = 0.0;
        Oscillator(size_t _type, double _tuning, double _amplitude, double _pulse_cycles);
        Oscillator(void) = default;
        size_t get_type(void) const { return type; }
        double get_amplitude(void) const { return amplitude; }
        double get_tuning(void) const { return tuning; }
        double get_pulse_cycle(void) const { return pulse_cycles; }
    private:
        size_t type = SQUARE;
        double tuning = 1.0;
        double amplitude = 1.0;
        double pulse_cycles = 0.5;
};

class Synth_Parameters {
public:
    Synth_Parameters(int sample_rate, std::vector<Oscillator> _oscillator_list);
    ~Synth_Parameters() = default;

    size_t get_osc_count(void) const { return oscillator_list.size(); }
    const Oscillator *const get_oscillator_at(size_t i) const;
    double get_attack_rate(void) const { return attack_rate; }
    double get_decay_rate(void) const { return decay_rate; }
    double get_release_rate(void) const { return release_rate; }

    double get_sustain(void) const { return SUS; }
    double get_attack(void) const { return ATK; }
    double get_decay(void) const { return DEC; }
    double get_release(void) const { return REL; }
    double get_svf_base_hz(void) const { return svf_filter_base_cutoff; }
    double get_svf_max_hz(void) const { return svf_filter_max_cutoff; }
    double get_svf_resonance(void) const { return svf_resonance; }

private:
    std::vector<Oscillator> oscillator_list;
    double attack_rate = 0.0;
    double decay_rate = 0.0;
    double release_rate = 0.0;
    unsigned int ATK = 800;
    unsigned int DEC = 900;
    unsigned int REL = 750;
    double SUS = 0.7;
    double svf_filter_base_cutoff = 1000.0;
    double svf_filter_max_cutoff = 4000.0;
    double svf_resonance = 0.707;
};

// Per voice state
class Oscillator_State {
public:
    static constexpr double phase_max = 1.0;
    Oscillator_State() = default;
    ~Oscillator_State() = default;
    void reset(void);
    void set_time(double _time) { time = _time; }
    void set_phase(double _phase) { phase = _phase; }
    double time_increment(double dt);
    double phase_increment(double inc);
    double get_phase(void) const { return phase; }
    double get_time(void) const { return time; }
    double run(const Oscillator *const osc, double freq, double sample_rate);

private:
    double phase = 0.0;
    double time = 0.0;
    std::array<Lfo, LFO_SIZE> lfo{}; 
};

class Voice {
public:
    static constexpr double ONE_POLE_HZ = 8000.0;
    static constexpr double dead_freq = 0.0;
    Voice(const Synth_Parameters& _cfg);
    ~Voice() = default;

    double apply_drive(double sample);
    double apply_gain(double sample);
    void reset_filters(void);
    bool new_note_reset(double _freq, double _velocity, unsigned int _midi_key);
    double state_variable_filter(double input, double cutoff, double dt, double Q);
    unsigned int get_state(void) const { return state; }
    void set_state(unsigned int _state) { state = _state; }
    void set_target_frequency(double f) { target_freq = f; }
    void set_prev_frequency(double f) { prev_freq = f; }
    void set_velocity(double v) { velocity = v; }
    void set_key(unsigned int _midi_key) { midi_key = _midi_key; }
    unsigned int get_key(void) const { return midi_key; }
    double get_target_frequency(void) const { return target_freq; }
    double get_prev_frequency(void) const  { return prev_freq; }
    double get_velocity(void) const { return velocity; }
    double env_mult(double sample);
    void set_real_frequency(double f) { real_freq = f; }
    double get_real_frequency(void) const { return real_freq; }
    void adsr(double dt);
    void reset_env(void) { env_value = 0.0; }
    double run(int sample_rate);
    double run_low_pass(double dt, double voice_sample);

    std::vector<Oscillator_State>& get_oscs(void) { return oscs; }

private:
    const Synth_Parameters& cfg;
    std::vector<Oscillator_State> oscs;
    std::array<double, INDEXES_SIZE> filter{};
    double target_freq = dead_freq;
    double prev_freq = dead_freq;
    double real_freq = dead_freq;
    double velocity = 0.0;
    unsigned int midi_key = 0;
    unsigned int state = OFF;
    unsigned int portamento_time_ms = 10;
    double env_value = 0.0;
};

class Synth {
public:
    static constexpr size_t volume_max = 10.0;
    static constexpr size_t volume_min = 0.0;
    static constexpr size_t voice_count = 16;
    static constexpr int sample_rate = 48000;
    static constexpr int channels = 1;
    Synth();
    ~Synth() = default;

    int get_sample_rate(void) const { return sample_rate; }
    int get_channels(void) const { return channels; }
    void run(size_t N, float *hardware_buffer);
    void voice_push_note(double freq, double velocity, unsigned int midi_key);
    void voice_send_kill_signal(unsigned int midi_key);
    double get_volume(void) const { return volume; }

private:
    double volume = 1.0;
    Synth_Parameters cfg;
    std::vector<Voice> voices;
    Compressor comp;
};

#endif
