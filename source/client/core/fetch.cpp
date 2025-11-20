#if defined(__EMSCRIPTEN__)
#include <emscripten/fetch.h>
#else
#include <thread>
#endif

#include <fstream>
#include <iostream>

#include <lucaria/core/fetch.hpp>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#include <android_native_app_glue.h>

namespace lucaria {
extern android_app* g_app; // declared in your window.cpp / main.cpp
}
#endif

namespace lucaria {
namespace {

    static std::optional<std::filesystem::path> _fetch_path = std::nullopt;

    static std::atomic<std::size_t> _fetches_waiting = 0;

    static void fetch_bytes_impl(const std::filesystem::path& file_path,
        std::function<void(std::vector<char>)> callback,
        bool persist)
    {
        std::filesystem::path _fetch_file_path = file_path;
#if !defined(__ANDROID__) // AND EMSCRIPTEN WITH PACKAGED ASSET DIR
        _fetch_file_path = _fetch_path ? (_fetch_path.value() / file_path) : file_path;
#endif

        _fetches_waiting++;
#if defined(__EMSCRIPTEN__)
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        std::strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        if (persist)
            attr.attributes |= EMSCRIPTEN_FETCH_PERSIST_FILE;

        auto* cb_heap = new std::function<void(std::vector<char>)>(std::move(callback));
        attr.userData = cb_heap;

        attr.onsuccess = [](emscripten_fetch_t* fetch) {
            std::vector<char> buf(fetch->data, fetch->data + fetch->numBytes); // one copy from fetch->data
            auto* cbp = static_cast<std::function<void(std::vector<char>)>*>(fetch->userData);
            (*cbp)(std::move(buf)); // move into user continuation
            delete cbp;
            _fetches_waiting--;
            emscripten_fetch_close(fetch);
        };
        attr.onerror = [](emscripten_fetch_t* fetch) {
            auto* cbp = static_cast<std::function<void(std::vector<char>)>*>(fetch->userData);
            // TODO: optional error callback; for now log
            std::fprintf(stderr, "fetch_bytes error: %s (%d)\n", fetch->statusText, fetch->status);
            delete cbp;
            emscripten_fetch_close(fetch);
            std::terminate();
        };

        emscripten_fetch(&attr, _fetch_file_path.c_str());
#else
        std::thread([_fetch_file_path, callback]() {
            detail::load_bytes(_fetch_file_path, callback);
            _fetches_waiting--;
        }).detach();
#endif
    }
}

void set_fetch_path(const std::filesystem::path& fetch_path)
{
    _fetch_path = fetch_path;
}

std::size_t get_fetches_waiting()
{
    return _fetches_waiting.load();
}

namespace detail {

    bytes_streambuf::bytes_streambuf(const std::vector<char>& data)
    {
        char* begin = const_cast<char*>(data.data());
        char* end = begin + data.size();
        setg(begin, begin, end);
    }

    bytes_stream::bytes_stream(const std::vector<char>& data)
        : std::istream(&_buffer)
        , _buffer(data)
    {
        this->setstate(std::ios::goodbit);
    }

    ozz_bytes_stream::ozz_bytes_stream(const std::vector<char>& data)
        : _bytes(data)
        , _position(0)
    {
    }

    bool ozz_bytes_stream::opened() const
    {
        // the stream is always "opened" when constructed
        return true;
    }

    std::size_t ozz_bytes_stream::Read(void* buffer, std::size_t size)
    {
        std::size_t remaining = _bytes.size() - _position;
        std::size_t to_read = std::min(size, remaining);
        std::memcpy(buffer, _bytes.data() + _position, to_read);
        _position += to_read;
        return to_read;
    }

    std::size_t ozz_bytes_stream::Write(const void* buffer, std::size_t size)
    {
        // not implemented since this is a read only stream
        return 0;
    }

    int ozz_bytes_stream::Seek(int offset, Origin origin)
    {
        int new_position = 0;
        switch (origin) {
        case kSet:
            new_position = offset;
            break;
        case kCurrent:
            new_position = static_cast<int>(_position + offset);
            break;
        case kEnd:
            new_position = static_cast<int>(_bytes.size() + offset);
            break;
        default:
            return -1;
        }
        if (new_position < 0 || static_cast<std::size_t>(new_position) > _bytes.size()) {
            return -1;
        }
        _position = new_position;
        return 0;
    }

    int ozz_bytes_stream::Tell() const
    {
        return static_cast<int>(_position);
    }

    std::size_t ozz_bytes_stream::Size() const
    {
        return _bytes.size();
    }

    void load_bytes(
        const std::filesystem::path& file_path,
        const std::function<void(const std::vector<char>&)>& callback)
    {
        std::string _path_str = file_path.string();

#if defined(__ANDROID__)
        AAssetManager* mgr = lucaria::g_app->activity->assetManager;
        AAsset* asset = AAssetManager_open(mgr, _path_str.c_str(), AASSET_MODE_STREAMING);
        if (!asset) {
            LUCARIA_RUNTIME_ERROR("open failed: " + _path_str)
        }
        const off_t len = AAsset_getLength(asset);
        std::vector<char> buffer(static_cast<size_t>(len));
        const int64_t read = AAsset_read(asset, buffer.data(), len);
        AAsset_close(asset);
        if (read != len) {
            LUCARIA_RUNTIME_ERROR("read failed: " + _path_str)
        }
        callback(buffer);

#else
        std::ifstream _fstream(file_path, std::ios::binary);
        if (!_fstream) {
            LUCARIA_RUNTIME_ERROR("open failed: " + _path_str)
        }
        _fstream.seekg(0, std::ios::end);
        const std::streamoff _size = _fstream.tellg();
        if (_size < 0) {
            LUCARIA_RUNTIME_ERROR("tellg failed: " + _path_str)
        }
        std::vector<char> _bytes(static_cast<std::size_t>(_size));
        _fstream.seekg(0, std::ios::beg);
        _fstream.read(_bytes.data(), _bytes.size());
        if (!_fstream) {
            LUCARIA_RUNTIME_ERROR("read failed: " + _path_str)
        }
        callback(std::move(_bytes));
#endif
    }

    void fetch_bytes(const std::filesystem::path& file_path,
        const std::function<void(const std::vector<char>&)>& callback,
        bool persist)
    {
        fetch_bytes_impl(
            file_path,
            [callback](std::vector<char> bytes) { callback(bytes); },
            persist);
    }

    void fetch_bytes(const std::vector<std::filesystem::path>& file_paths,
        const std::function<void(const std::vector<std::vector<char>>&)>& callback,
        bool persist)
    {
        const std::size_t _size = file_paths.size();

        if (_size == 0) {
            static const std::vector<std::vector<char>> _empty;
            callback(_empty);
            return;
        }

        std::shared_ptr<std::vector<std::vector<char>>> _shared_slots = std::make_shared<std::vector<std::vector<char>>>(_size);
        std::shared_ptr<std::atomic<std::size_t>> _shared_pending = std::make_shared<std::atomic<std::size_t>>(_size);

        for (std::size_t _index = 0; _index < _size; ++_index) {

            fetch_bytes_impl(
                file_paths[_index],
                [_index, _shared_slots, _shared_pending, callback](std::vector<char> bytes) {
                    (*_shared_slots)[_index] = std::move(bytes);

                    if (_shared_pending->fetch_sub(1, std::memory_order_acq_rel) == 1) {
                        callback(*_shared_slots);
                    }
                },
                persist);
        }
    }

}

}
