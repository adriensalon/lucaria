#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <vorbis/vorbisfile.h>

#include <core/fetch.hpp>
#include <core/sound.hpp>

namespace detail {

struct VorbisStream {
    std::istringstream& stream;
    std::vector<char> buffer;

    VorbisStream(std::istringstream& dataStream) : stream(dataStream) {
        // Copy data from istringstream to buffer
        stream.seekg(0, std::ios::end);
        size_t size = stream.tellg();
        buffer.resize(size);
        stream.seekg(0, std::ios::beg);
        stream.read(buffer.data(), size);
        // Reset stream to read from the buffer
        stream.str(std::string(buffer.begin(), buffer.end()));
    }
};


// Callback functions for OggVorbis to read from std::istringstream
size_t read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {
    VorbisStream* vs = static_cast<VorbisStream*>(datasource);
    vs->stream.read(static_cast<char*>(ptr), size * nmemb);
    return vs->stream.gcount();
}

int seek_func(void* datasource, ogg_int64_t offset, int whence) {
    VorbisStream* vs = static_cast<VorbisStream*>(datasource);
    std::istringstream::pos_type pos;

    switch (whence) {
        case SEEK_SET: pos = offset; break;
        case SEEK_CUR: pos = vs->stream.tellg() + std::istringstream::pos_type(offset); break;
        case SEEK_END: pos = vs->buffer.size() - offset; break;
        default: return -1;
    }

    vs->stream.clear();  // Clear any error state
    vs->stream.seekg(pos);

    return vs->stream.good() ? 0 : -1;
}

long tell_func(void* datasource) {
    VorbisStream* vs = static_cast<VorbisStream*>(datasource);
    return static_cast<long>(vs->stream.tellg());
}

int close_func(void* datasource) {
    // No actual resource to close in this example
    return 0;
}

static std::unordered_map<std::string, std::promise<std::shared_ptr<sound_ref>>> promises;

}

sound_ref::sound_ref(sound_ref&& other)
{
    *this = std::move(other);
}

sound_ref& sound_ref::operator=(sound_ref&& other)
{
    // todo
    return *this;
}

sound_ref::~sound_ref()
{
    if (_is_instanced) {
        // todo
    }
}

sound_ref::sound_ref(const audio_data& data)
{
    // todo
}

glm::uint sound_ref::get_id() const
{
    return _buffer_id;
}

audio_data load_audio_data(std::istringstream& audio_stream)
{
    audio_data _data;
    detail::VorbisStream vorbisStream(audio_stream);
    OggVorbis_File vorbis;

    ov_callbacks callbacks;
    callbacks.read_func = detail::read_func;
    callbacks.seek_func = detail::seek_func;
    callbacks.tell_func = detail::tell_func;
    callbacks.close_func = detail::close_func;

    if (ov_open_callbacks(&vorbisStream, &vorbis, nullptr, 0, callbacks) != 0) {
        // std::cerr << "Invalid Ogg file '" << argv[1] << "'." << std::endl;
        // return -1;
    }

    // Print sound information
    vorbis_info* info = ov_info(&vorbis, -1);
    std::cout << "Ogg file " << info->rate << " Hz, " << info->channels << " channels, " << info->bitrate_nominal / 1024 << " kbit/s." << std::endl;

    // Read the entire sound stream
    unsigned char buf[4096];
    while (true) {
        int section = 0;
        long bytes = ov_read(&vorbis, reinterpret_cast<char*>(buf), sizeof(buf), 0, 2, 1, &section);
        std::cout << "reading " << bytes << std::endl;
        if (bytes <= 0)  // End of file or error
            break;
    }

    // Close sound file
    ov_clear(&vorbis);

    return _data;
}

std::shared_future<std::shared_ptr<sound_ref>> fetch_sound(const std::filesystem::path& sound_path)
{
    std::promise<std::shared_ptr<sound_ref>>& _promise = detail::promises[sound_path.string()];
    fetch_file(sound_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<sound_ref>(load_audio_data(stream))));
    });
    return _promise.get_future();
}
