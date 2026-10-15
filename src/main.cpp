#include "../inc/keyboard.hpp"
#include "../inc/util.hpp"
#include "../inc/audio.hpp"
#include "../inc/synth.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>

#include <SDL3/SDL.h>
#include <RtMidi.h>

// A compressed sample rate that was used in some hardware GBA games.
constexpr int SAMPLE_RATE = 13379;
constexpr int CHANNELS = 2;

static bool sdl_init(void);
static void sdl_deinit(void);
static void sdl_print_version(void);
static bool sdl_wait_quit(void);

static std::unique_ptr<MidiContext> create_midi_context();

int main(int argc, char **argv){
    if(!sdl_init()){
        return EXIT_FAILURE;
    }
    sdl_print_version();

    std::unique_ptr<MidiContext> midi_context = create_midi_context();
    if(!midi_context){
        return 1;
    }

    Synth synth;
    Audio_Sys audio(CHANNELS, SAMPLE_RATE);
    audio.open(&synth);

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

static std::unique_ptr<MidiContext> create_midi_context(){
    try {
        return std::make_unique<MidiContext>();
    } catch (const std::runtime_error& error) {
        std::cerr << "** " << error.what() << " ** { Caught at create_midi_context() } " << std::endl;
        return nullptr;
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
