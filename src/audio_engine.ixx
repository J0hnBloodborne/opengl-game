module;

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <iostream>
#include <map>
#include <string>

export module audio_engine;

export class AudioEngine {
public:
    AudioEngine() {
        ma_result result;
        result = ma_engine_init(NULL, &engine);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to initialize audio engine." << std::endl;
        }
    }

    ~AudioEngine() {
        ma_engine_uninit(&engine);
    }

    void Play(const std::string& soundFile) {
        ma_engine_play_sound(&engine, soundFile.c_str(), NULL);
    }

private:
    ma_engine engine;
};
