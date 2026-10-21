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
    FILTER_PRE = 0,
    FILTER_POST,
    INDEXES_SIZE
};

class Voice;
class Oscillator_State;
class Oscillator_Configuration;
class Synth;
class Lfo;

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
private:
    double phase = 0.0;
    double rate = 6.0;
    double depth = 3.0;
};

class Oscillator_Configuration {
public:
    Oscillator_Configuration(int sample_rate, std::vector<size_t> _types);
    ~Oscillator_Configuration() = default;

    size_t get_osc_count(void) const { return types.size(); }
    size_t get_type_at(size_t i) const;
    double get_attack_rate(void) const { return attack_rate; }
    double get_decay_rate(void) const { return decay_rate; }
    double get_release_rate(void) const { return release_rate; }

    double get_gain(void) const { return GAIN; }
    double get_sustain(void) const { return SUS; }
    double get_attack(void) const { return ATK; }
    double get_decay(void) const { return DEC; }
    double get_release(void) const { return REL; }
    double get_cycle(void) const { return CYCLE; }

private:
    std::vector<size_t> types;
    double attack_rate = 0.0;
    double decay_rate = 0.0;
    double release_rate = 0.0;
    unsigned int ATK = 100;
    unsigned int DEC = 250;
    unsigned int REL = 500;
    double SUS = 0.7;
    double GAIN = 1.5;
    double CYCLE = 0.25;
};

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
    Lfo& get_lfo1(void) { return lfo1; }
    double run(const Oscillator_Configuration& cfg, size_t i, double freq, double sample_rate);

private:
    double phase = 0.0;
    double time = 0.0;
    Lfo lfo1;
    Lfo lfo2;
    Lfo lfo3;
};

class Voice {
public:
    static constexpr double dead_freq = 0.0;
    Voice(const Oscillator_Configuration& _cfg);
    ~Voice() = default;

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
    double run(double dt, int sample_rate);

    std::vector<Oscillator_State>& get_oscs(void) { return oscs; }

private:
    const Oscillator_Configuration& cfg;
    std::vector<Oscillator_State> oscs;
    std::array<double, INDEXES_SIZE> filter{};
    double target_freq = dead_freq;
    double prev_freq = dead_freq;
    double real_freq = dead_freq;
    double velocity = 0.0;
    unsigned int midi_key = 0;
    unsigned int state = OFF;
    unsigned int portamento_time_ms = 200;
    double env_value = 0.0;
};

class Synth {
public:
    static constexpr size_t voice_count = 8;
    static constexpr int sample_rate = 48000;
    static constexpr int channels = 1;
    Synth();
    ~Synth() = default;

    int get_sample_rate(void) const { return sample_rate; }
    int get_channels(void) const { return channels; }
    void run(size_t N, float *hardware_buffer);
    void voice_push_note(double freq, double velocity, unsigned int midi_key);
    void voice_send_kill_signal(unsigned int midi_key);
private:
    Oscillator_Configuration osc_config;
    std::vector<Voice> voices;
};

#endif
