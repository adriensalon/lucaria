#include <filesystem>
#include <iostream>

#include <AL/al.h>
#include <AL/alc.h>
// #include <minimp3/minimp3.h>
#include <emscripten/html5.h>
#include <glm/glm.hpp>

namespace detail {

    extern void emscripten_assert(EMSCRIPTEN_RESULT result);

    static bool has_webaudio;
    static bool is_webaudio_setup = false;

    void create_openal_context_if_needed()
    {
        // https://emscripten.org/docs/porting/Audio.html
        if (is_webaudio_setup) {
            return;
        }
        ALCdevice* _webaudio_device = alcOpenDevice(NULL);
        if (!_webaudio_device) {
            std::cerr << "Impossible to create an OpenAL device" << std::endl;
            has_webaudio = false;
            is_webaudio_setup = true;
            return;
        }
        ALCcontext* _webaudio_context = alcCreateContext(_webaudio_device, NULL);
        if (!_webaudio_context) {
            std::cerr << "Impossible to create an OpenAL context" << std::endl;
            has_webaudio = false;
            is_webaudio_setup = true;
            return;
        }
        if (!alcMakeContextCurrent(_webaudio_context)) {
            std::cerr << "Impossible to use an OpenAL context" << std::endl;
            has_webaudio = false;
            is_webaudio_setup = true;
            return;
        }
        bool _is_float32_supported = (alIsExtensionPresent("AL_EXT_float32") == AL_TRUE);
        if (!_is_float32_supported) {
            std::cerr << "OpenAL extension 'AL_EXT_float32' is not supported" << std::endl;
            return;
        }
        has_webaudio = true;
        is_webaudio_setup = true;
    }

    void destroy_openal_context()
    {
        ALCcontext* _webaudio_context = alcGetCurrentContext();
        ALCdevice* _webaudio_device = alcGetContextsDevice(_webaudio_context);
        alcMakeContextCurrent(NULL);
        alcDestroyContext(_webaudio_context);
        alcCloseDevice(_webaudio_device);
    }



    static ALuint left_source_id = 0;
    static ALuint right_source_id = 0;
}

void audio_assert()
{
#if DEBUG
    std::string _reason;
    ALenum _al_error = alGetError();
    if (_al_error != AL_NO_ERROR) {
        if (_al_error == AL_INVALID_NAME)
            _reason = "invalid name";
        else if (_al_error == AL_INVALID_ENUM)
            _reason = " invalid enum";
        else if (_al_error == AL_INVALID_VALUE)
            _reason = " invalid value";
        else if (_al_error == AL_INVALID_OPERATION)
            _reason = " invalid operation";
        else if (_al_error == AL_OUT_OF_MEMORY)
            _reason = "out of memory";
        std::cerr << "Invalid OpenAL result '" << _reason << "'" << std::endl;
        std::terminate();
    }
#endif
}


void load_audio_mp3(const std::filesystem::path& path)
{

}

void play_audio()
{

}

void stop_audio()
{

}

void set_audio_positions(const glm::vec3 left, const glm::vec3 right)
{
    alSource3f(detail::left_source_id, AL_POSITION, left.x, left.y, left.z);
    alSource3f(detail::right_source_id, AL_POSITION, right.x, right.y, right.z);
    audio_assert();
}

void set_audio_directions(const glm::vec3 left, const glm::vec3 right)
{
    alSource3f(detail::left_source_id, AL_DIRECTION, left.x, left.y, left.z);
    alSource3f(detail::right_source_id, AL_DIRECTION, right.x, right.y, right.z);
    audio_assert();
}

void set_audio_velocities(const glm::vec3 left, const glm::vec3 right)
{
    alSource3f(detail::left_source_id, AL_VELOCITY, left.x, left.y, left.z);
    alSource3f(detail::right_source_id, AL_VELOCITY, right.x, right.y, right.z);
    audio_assert();
}

void set_audio_loop(const bool loop)
{

}

void set_audio_gain(const float gain, const float min, const float max)
{

}

void set_audio_pitch(const float pitch)
{

}
