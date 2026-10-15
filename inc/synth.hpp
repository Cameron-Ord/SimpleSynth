#ifndef SYNTH_HPP
#define SYNTH_HPP
#include <vector>

enum Wave_Indexes : size_t{
    SINE = 0,
    SAW,
    SQUARE,
    TRIANGLE,
    PULSE,
};

class Voice;
class Oscillator_State;

class Oscillator_Configuration {
public:
    static constexpr double ATK = 0.1;
    static constexpr double DEC = 0.1;
    static constexpr double SUS = 0.1;
    static constexpr double REL = 0.1;

    Oscillator_Configuration();
    ~Oscillator_Configuration() = default;
private:
    size_t oscillator_type = SINE;
};

class Synth {
public:
    static constexpr size_t voice_count = 4;
    Synth();
    ~Synth() = default;

    void run(void);

private:
    Oscillator_Configuration osc_config;
};

class Voice {
public:
    static constexpr size_t osc_max = 1;
    Voice();
    ~Voice() = default;
    bool query_active(void) const { return active; }

private:
    std::vector<Oscillator_State> oscs;
    bool active = false;
};

class Oscillator_State {
public:
    static constexpr double phase_max = 1.0;
    Oscillator_State();
    ~Oscillator_State() = default;
    void reset(void);
    double time_increment(double dt);
    double phase_increment(double inc);

private:
    double phase = 0.0;
    double time = 0.0;
};


#endif
