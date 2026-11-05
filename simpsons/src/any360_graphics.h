#pragma once

// Test branch any360-graphics-test: the GPU through Any360 (github.com/GaetanVigner/Any360) behind the any360_gpu
// setting. Only built when CMake is given SIMPSONS_ANY360_DIR (the definition below says so).
namespace simpsons {

// Binds the GPU to Any360 when any360_gpu is on; logs and keeps the runtime's GPU when it cannot.
void StartAny360GraphicsIfAsked();
// Stops Any360's threads before the runtime goes away (and writes the any360_gpu_profile samples).
void StopAny360Graphics();

}  // namespace simpsons
