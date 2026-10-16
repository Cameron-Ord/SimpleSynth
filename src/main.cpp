#include "../inc/keyboard.hpp"
#include "../inc/util.hpp"
#include "../inc/audio.hpp"
#include "../inc/synth.hpp"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>

#include <SDL3/SDL.h>
#include <RtMidi.h>
#include <stdexcept>



// A compressed sample rate that was used in some hardware GBA games.
static bool sdl_init(void);
static void sdl_deinit(void);
static void sdl_print_version(void);
static bool sdl_wait_quit(void);

static std::unique_ptr<MidiContext> create_midi_context(void *const error_userdata, void *const synth_userdata);
static std::unique_ptr<Synth> create_synth_context(void);
static void open_audio(Audio_Sys& a, void *const userdata);

int main(int argc, char **argv){
    if(!sdl_init()){
        return EXIT_FAILURE;
    }
    sdl_print_version();

    const std::unique_ptr<Synth> synth = create_synth_context();
    const std::unique_ptr<MidiContext> midi_context = create_midi_context(nullptr, synth.get());

    Audio_Sys audio(synth->get_channels(), synth->get_sample_rate());
    open_audio(audio, synth.get());

    bool looping = true;
    while(looping){
        if(sdl_wait_quit()){
            looping = false;
        }
    }

    print_charstr("Quiting!");
    audio.close();
    sdl_deinit();
    return EXIT_SUCCESS;
}

static void open_audio(Audio_Sys& a, void *const userdata) {
    try {
        a.open(userdata);
    } catch (const std::runtime_error& error) {
        std::cerr << "** " << error.what() << " ** { Caught at open_audio() } " << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

static std::unique_ptr<Synth> create_synth_context(void){
    try {
        return std::make_unique<Synth>();
    } catch (const std::bad_alloc& error) {
        std::cerr << "** " << error.what() << " ** { Caught at create_synth_context() } " << std::endl;
        std::exit(EXIT_FAILURE);
    // extreme paranoia i guess
    } catch (const std::exception& error){
        std::cerr << "** " << error.what() << " ** { Caught at create_synth_context() } " << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

static std::unique_ptr<MidiContext> create_midi_context(void *const error_userdata, void *const synth_userdata){
    try {
        return std::make_unique<MidiContext>(error_userdata, synth_userdata);
    } catch (const std::runtime_error& error) {
        std::cerr << "** " << error.what() << " ** { Caught at create_midi_context() } " << std::endl;
        std::exit(EXIT_FAILURE);
    } catch (const std::bad_alloc& error) {
        std:: cerr << "** Out of memory **" << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
// Headless SDL app can't read keyboard inputs without a focused 
// rendering context 

// So any event that kills the process would be a way to end the program
// IE: terminal Ctrl-C interupt

// This program is a very simple synth meant to just sit in the background without 
// tweakable parameters or a GUI so I will just be leaving it this way.

// If at any point I wish to create a GUI for this synth and changeable parameters, 
// it will be a seperate repo, but It will originate from here.
static bool sdl_wait_quit(void){
    SDL_Event event;
    SDL_WaitEvent(&event);
    switch(event.type){
        default: break;
        case SDL_EVENT_QUIT: {
            return true;
        } break;
    }
    return false;
}

void print_charstr(const char *msg) {
    std::cerr << msg << std::endl;
}

static void sdl_print_version(void){
    const int compiled = SDL_VERSION;
    const int linked = SDL_GetVersion();
    SDL_Log(
        "\n{ Compiled against SDL version %d.%d.%d }\n",
        SDL_VERSIONNUM_MAJOR(compiled),
        SDL_VERSIONNUM_MINOR(compiled),
        SDL_VERSIONNUM_MICRO(compiled)
    );

    SDL_Log(
        "{ Linking against SDL version %d.%d.%d }\n",
        SDL_VERSIONNUM_MAJOR(linked),
        SDL_VERSIONNUM_MINOR(linked),
        SDL_VERSIONNUM_MICRO(linked)
    );
}

static void sdl_deinit(void){
    SDL_Quit();
}

static bool sdl_init(void){
    if(!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS)){
        print_charstr(SDL_GetError());
        return false;
    }
    return true;
}
