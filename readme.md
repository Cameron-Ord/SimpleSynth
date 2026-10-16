# Simple Portamento Synth
A really simple freakin' synth. Relatively barebones and a decent starting point (for me) to jump off of.

I'm using SDL3 to handle audio, and RtMidi for midi control with midi keyboards. This keeps things easy for me(and cross platform).

Building on windows using cmake and ninja at the moment. Haven't put much time into streamlining the build process and making sure it builds cross platform yet honestly.

## Building
```cmake -B build -G Ninja```
```cmake --build build```