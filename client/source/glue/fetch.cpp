#include <filesystem>
#include <iostream>
#include <unordered_map>
#include <vector>


#include <emscripten/fetch.h>

#include <glue/fetch.hpp>

using fetch_callback = std::function<void(std::istringstream&)>;
using multiple_fetch_callback = std::function<void(std::size_t, std::size_t, std::istringstream&)>;

namespace detail {

std::size_t fetch_total = 0;
std::size_t fetch_completed = 0;
std::size_t fetch_failed = 0;
std::unordered_map<std::string, fetch_callback> fetch_requests;
std::unordered_map<std::string, multiple_fetch_callback> multiple_fetch_requests;

std::size_t compute_hash(std::size_t lhs, std::size_t rhs)
{
    return lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
}

void on_fetch_success(emscripten_fetch_t* fetch)
{
    fetch_completed++;
    std::cout << "Successfully fetched " << fetch->numBytes << " bytes from " << fetch->url << std::endl;
    std::vector<char> _data(fetch->data, fetch->data + fetch->numBytes);
// #if LUCARIA_JSON
//     std::istringstream _stream(std::string(_data.begin(), _data.end()));
// #else
    std::istringstream _stream(std::string(_data.begin(), _data.end()), std::ios::binary);
// #endif
    fetch_requests[fetch->url](_stream);
    emscripten_fetch_close(fetch);
}

void on_multiple_fetch_success(emscripten_fetch_t* fetch)
{
    fetch_completed++;
    std::cout << "Successfully fetched MULTI " << fetch->numBytes << " bytes from " << fetch->url << std::endl;
    std::vector<char> _data(fetch->data, fetch->data + fetch->numBytes);
// #if LUCARIA_JSON
//     std::istringstream _stream(std::string(_data.begin(), _data.end()));
// #else
    std::istringstream _stream(std::string(_data.begin(), _data.end()), std::ios::binary);
// #endif
    std::pair<std::size_t, std::size_t>* _user_data = static_cast<std::pair<std::size_t, std::size_t>*>(fetch->userData);
    const std::size_t _index = _user_data->first;
    const std::size_t _total = _user_data->second;
    std::cout << "fetched " << fetch->url << "  " << std::to_string(_index) << "/" << std::to_string(_total) << std::endl;
    multiple_fetch_requests[fetch->url](_index, _total, _stream);
    delete _user_data;
    emscripten_fetch_close(fetch);
}

void on_fetch_error(emscripten_fetch_t* fetch)
{
    fetch_failed++;
    std::cout << "Failed to fetch " << fetch->url << " with status " << fetch->status << std::endl;
    emscripten_fetch_close(fetch);
}

void on_multiple_fetch_error(emscripten_fetch_t* fetch)
{
    fetch_failed++;
    std::cout << "Failed to fetch " << fetch->url << " with status " << fetch->status << std::endl;
    std::pair<std::size_t, std::size_t>* _user_data = static_cast<std::pair<std::size_t, std::size_t>*>(fetch->userData);
    delete _user_data;
    emscripten_fetch_close(fetch);
}

}

bool ozz_istringstream::opened() const {
    return _stream.good();
}

std::size_t ozz_istringstream::Read(void* buffer, std::size_t size) {
    _stream.read(static_cast<char*>(buffer), size);
    return _stream.gcount();
}

std::size_t ozz_istringstream::Write(const void* buffer, std::size_t size) {
    return 0;
}

int ozz_istringstream::Tell() const {
    return static_cast<int>(_stream.tellg());
}

int ozz_istringstream::Seek(int offset, Origin origin) {
    std::ios_base::seekdir _direction;        
    switch (origin) {
        case ozz::io::Stream::kCurrent:
            _direction = std::ios::cur;
            break;
        case ozz::io::Stream::kEnd:
            _direction = std::ios::end;
            break;
        case ozz::io::Stream::kSet:
            _direction = std::ios::beg;
            break;
        default:
            return -1;
    }    
    _stream.seekg(offset, _direction);
    if (_stream.fail()) {
        return -1;
    }
    return static_cast<int>(_stream.tellg());
}

std::size_t ozz_istringstream::Size() const {
    auto _current = _stream.tellg();
    _stream.seekg(0, std::ios::end);
    auto _size = _stream.tellg();
    _stream.seekg(_current);
    return _size;
}

std::size_t compute_hash_files(const std::vector<std::filesystem::path>& files)
{
    std::size_t _combined_hash = 0;
    for (const std::filesystem::path& _file : files) {
        _combined_hash = detail::compute_hash(_combined_hash, std::hash<std::string> {}(_file));
    }
    return _combined_hash;
}

void fetch_file(const std::string& url, const fetch_callback& callback, const bool persist)
{
    detail::fetch_total++;
    detail::fetch_requests[url] = callback;
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    if (persist) {
        attr.attributes |= EMSCRIPTEN_FETCH_PERSIST_FILE;
    }
    attr.onsuccess = detail::on_fetch_success;
    attr.onerror = detail::on_fetch_error;
    emscripten_fetch(&attr, url.c_str());
}

void fetch_files(const std::vector<std::filesystem::path>& files, const std::function<void(const std::size_t, const std::size_t, std::istringstream&)>& callback, const bool persist)
{
    const std::size_t _size = files.size();
    detail::fetch_total += _size;
    for (std::size_t _index = 0; _index < _size; ++_index) {
        std::pair<std::size_t, std::size_t>* _user_data = new std::pair<std::size_t, std::size_t>(_index, _size);
        const std::string _file = files[_index].generic_string();
        detail::multiple_fetch_requests[_file] = callback;
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        if (persist) {
            attr.attributes |= EMSCRIPTEN_FETCH_PERSIST_FILE;
        }
        attr.onsuccess = detail::on_multiple_fetch_success;
        attr.onerror = detail::on_multiple_fetch_error;
        attr.userData = _user_data;
        emscripten_fetch(&attr, _file.c_str());
    }
}

std::size_t get_fetches_total()
{
    return detail::fetch_total;
}

std::size_t get_fetches_completed()
{
    return detail::fetch_completed;
}

std::size_t get_fetches_failed()
{
    return detail::fetch_failed;
}