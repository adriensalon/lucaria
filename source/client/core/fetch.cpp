#if defined(__EMSCRIPTEN__)
#include <emscripten/fetch.h>
#else
#include <fstream>
#include <mutex>
#include <thread>
#endif

#include <lucaria/core/fetch.hpp>
#include <lucaria/core/load.hpp>

namespace detail {

static std::size_t fetch_total = 0;
static std::size_t fetch_completed = 0;
static std::size_t fetch_failed = 0;
static std::unordered_map<std::string, fetch_callback> fetch_requests;
static std::unordered_map<std::string, fetch_raw_callback> fetch_raw_requests;
static std::unordered_map<std::string, multiple_fetch_callback> multiple_fetch_requests;
static std::unordered_map<std::string, multiple_fetch_raw_callback> multiple_fetch_raw_requests;

#if !defined(__EMSCRIPTEN__)
static std::mutex fetch_mutex;
#endif

#if defined(__EMSCRIPTEN__)
static void on_fetch_success(emscripten_fetch_t* fetch)
{
    fetch_completed++;
    std::cout << "Successfully fetched " << fetch->numBytes << " bytes from " << fetch->url << std::endl;
    std::vector<char> _data(fetch->data, fetch->data + fetch->numBytes);
#if LUCARIA_JSON
    std::istringstream _stream(std::string(_data.begin(), _data.end()));
#else
    std::istringstream _stream(std::string(_data.begin(), _data.end()), std::ios::binary);
#endif
    fetch_requests[fetch->url](_stream);
    emscripten_fetch_close(fetch);
}

static void on_fetch_raw_success(emscripten_fetch_t* fetch)
{
    fetch_completed++;
    std::cout << "Successfully fetched " << fetch->numBytes << " bytes from " << fetch->url << std::endl;
    std::vector<char> _data(fetch->data, fetch->data + fetch->numBytes);
    fetch_raw_requests[fetch->url](_data);
    emscripten_fetch_close(fetch);
}

static void on_multiple_fetch_success(emscripten_fetch_t* fetch)
{
    fetch_completed++;
    std::cout << "Successfully fetched MULTI " << fetch->numBytes << " bytes from " << fetch->url << std::endl;
    std::vector<char> _data(fetch->data, fetch->data + fetch->numBytes);
#if LUCARIA_JSON
    std::istringstream _stream(std::string(_data.begin(), _data.end()));
#else
    std::istringstream _stream(std::string(_data.begin(), _data.end()), std::ios::binary);
#endif
    std::pair<std::size_t, std::size_t>* _user_data = static_cast<std::pair<std::size_t, std::size_t>*>(fetch->userData);
    const std::size_t _index = _user_data->first;
    const std::size_t _total = _user_data->second;
    // std::cout << "fetched " << fetch->url << "  " << std::to_string(_index) << "/" << std::to_string(_total) << std::endl;
    multiple_fetch_requests[fetch->url](_index, _total, _stream);
    delete _user_data;
    emscripten_fetch_close(fetch);
}

static void on_multiple_fetch_raw_success(emscripten_fetch_t* fetch)
{
    fetch_completed++;
    std::cout << "Successfully fetched MULTI " << fetch->numBytes << " bytes from " << fetch->url << std::endl;
    std::vector<char> _data(fetch->data, fetch->data + fetch->numBytes);
    std::pair<std::size_t, std::size_t>* _user_data = static_cast<std::pair<std::size_t, std::size_t>*>(fetch->userData);
    const std::size_t _index = _user_data->first;
    const std::size_t _total = _user_data->second;
    // std::cout << "fetched " << fetch->url << "  " << std::to_string(_index) << "/" << std::to_string(_total) << std::endl;
    multiple_fetch_raw_requests[fetch->url](_index, _total, _data);
    delete _user_data;
    emscripten_fetch_close(fetch);
}

static void on_fetch_error(emscripten_fetch_t* fetch)
{
    fetch_failed++;
    std::cout << "Failed to fetch " << fetch->url << " with status " << fetch->status << std::endl;
    emscripten_fetch_close(fetch);
}

static void on_multiple_fetch_error(emscripten_fetch_t* fetch)
{
    fetch_failed++;
    std::cout << "Failed to fetch " << fetch->url << " with status " << fetch->status << std::endl;
    std::pair<std::size_t, std::size_t>* _user_data = static_cast<std::pair<std::size_t, std::size_t>*>(fetch->userData);
    delete _user_data;
    emscripten_fetch_close(fetch);
}
#else
static void read_file(const std::filesystem::path& file, const fetch_callback& callback) {
    std::ifstream input(file, std::ios::binary);
    if (input) {
        fetch_completed++;
        std::ostringstream buffer;
        buffer << input.rdbuf();
        std::istringstream stream(buffer.str());
        callback(stream);
        std::cout << "Successfully fetched " << buffer.str().size() << " bytes from " << file << std::endl;
    } else {
        fetch_failed++;
        std::cerr << "Failed to fetch file: " << file << std::endl;
    }
}

static void read_file_raw(const std::filesystem::path& file, const fetch_raw_callback& callback) {
    std::ifstream input(file, std::ios::binary);
    if (input) {
        fetch_completed++;
        std::vector<char> data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        callback(data);
        std::cout << "Successfully fetched " << data.size() << " bytes from " << file << std::endl;
    } else {
        fetch_failed++;
        std::cerr << "Failed to fetch file: " << file << std::endl;
    }
}

#endif

}

void fetch_file(const std::filesystem::path& file, const fetch_callback& callback, const bool persist)
{
    detail::fetch_total++;
#if defined(__EMSCRIPTEN__)
    detail::fetch_requests[file.string()] = callback;
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    if (persist) {
        attr.attributes |= EMSCRIPTEN_FETCH_PERSIST_FILE;
    }
    attr.onsuccess = detail::on_fetch_success;
    attr.onerror = detail::on_fetch_error;
    emscripten_fetch(&attr, file.c_str());
#else
    // std::thread([file, callback]() {
    //     std::lock_guard<std::mutex> lock(detail::fetch_mutex);
    //     detail::read_file(file, callback);
    // }).detach();
    if (!std::filesystem::exists(file)) {
        std::cout << "Impossible to find file " << file << std::endl;
    }
    detail::read_file(file, callback);
#endif
}

void fetch_file(const std::filesystem::path& file, const fetch_raw_callback& callback, const bool persist)
{
    detail::fetch_total++;
#if defined(__EMSCRIPTEN__)
    detail::fetch_raw_requests[file.string()] = callback;
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    if (persist) {
        attr.attributes |= EMSCRIPTEN_FETCH_PERSIST_FILE;
    }
    attr.onsuccess = detail::on_fetch_raw_success;
    attr.onerror = detail::on_fetch_error;
    emscripten_fetch(&attr, file.c_str());
#else
    // std::thread([file, callback]() {
    //     std::lock_guard<std::mutex> lock(detail::fetch_mutex);
    //     detail::read_file_raw(file, callback);
    // }).detach();
    detail::read_file_raw(file, callback);
#endif
}

void fetch_files(const std::vector<std::filesystem::path>& files, const multiple_fetch_callback& callback, const bool persist)
{
    const std::size_t _size = files.size();
    detail::fetch_total += _size;
    for (std::size_t _index = 0; _index < _size; ++_index) {
        const std::filesystem::path& _file = files[_index];
#if defined(__EMSCRIPTEN__)
        std::pair<std::size_t, std::size_t>* _user_data = new std::pair<std::size_t, std::size_t>(_index, _size);
        detail::multiple_fetch_requests[_file.string()] = callback;
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
#else   
        // std::thread([_file, _index, _size, callback]() {
        //     std::lock_guard<std::mutex> lock(detail::fetch_mutex);
        //     std::ifstream input(_file, std::ios::binary);
        //     if (input) {
        //         detail::fetch_completed++;
        //         std::ostringstream buffer;
        //         buffer << input.rdbuf();
        //         std::istringstream stream(buffer.str());
        //         callback(_index, _size, stream);
        //     } else {
        //         detail::fetch_failed++;
        //         std::cerr << "Failed to fetch file: " << _file << std::endl;
        //     }
        // }).detach();
        std::ifstream input(_file, std::ios::binary);
        if (input) {
            detail::fetch_completed++;
            std::ostringstream buffer;
            buffer << input.rdbuf();
            std::istringstream stream(buffer.str());
            callback(_index, _size, stream);
            std::cout << "Successfully fetched " << buffer.str().size() << " bytes from " << _file << std::endl;
        }
#endif
    }
}

void fetch_files(const std::vector<std::filesystem::path>& files, const multiple_fetch_raw_callback& callback, const bool persist)
{
    const std::size_t _size = files.size();
    detail::fetch_total += _size;
    for (std::size_t _index = 0; _index < _size; ++_index) {
        const std::filesystem::path& _file = files[_index];
#if defined(__EMSCRIPTEN__)
        std::pair<std::size_t, std::size_t>* _user_data = new std::pair<std::size_t, std::size_t>(_index, _size);
        detail::multiple_fetch_raw_requests[_file.string()] = callback;
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        if (persist) {
            attr.attributes |= EMSCRIPTEN_FETCH_PERSIST_FILE;
        }
        attr.onsuccess = detail::on_multiple_fetch_raw_success;
        attr.onerror = detail::on_multiple_fetch_error;
        attr.userData = _user_data;
        emscripten_fetch(&attr, _file.c_str());
#else   
        // std::thread([_file, _index, _size, callback]() {
        //     std::lock_guard<std::mutex> lock(detail::fetch_mutex);
        //     std::ifstream input(_file, std::ios::binary);
        //     if (input) {
        //         detail::fetch_completed++;
        //         std::vector<char> data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        //         callback(_index, _size, data);
        //     } else {
        //         detail::fetch_failed++;
        //         std::cerr << "Failed to fetch file: " << _file << std::endl;
        //     }
        // }).detach();
        std::ifstream input(_file, std::ios::binary);
        if (input) {
            detail::fetch_completed++;
            std::vector<char> data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
            callback(_index, _size, data);
            std::cout << "Successfully fetched " << data.size() << " bytes from " << _file << std::endl;
        }
#endif
    }
}

// std::size_t get_fetches_total()
// {
//     return detail::fetch_total;
// }

// std::size_t get_fetches_completed()
// {
//     return detail::fetch_completed;
// }

// std::size_t get_fetches_failed()
// {
//     return detail::fetch_failed;
// }

std::size_t get_fetches_waiting()
{
    return detail::fetch_container_updaters.size();
}

// void reset_fetch_counters()
// {
// #if LUCARIA_DEBUG
//     if ((detail::fetch_total != detail::fetch_completed) && (detail::fetch_failed != 0)) {
//         std::cout << "Impossible to reset fetch counters." << std::endl;
//         std::terminate();
//     }
// #endif
//     detail::fetch_total = 0;
//     detail::fetch_completed = 0;
//     detail::fetch_failed = 0;
// }

void wait_one_fetched_container()
{
    // std::cout << "fetxh size = " << detail::fetch_container_updaters.size() << std::endl;
    if (detail::fetch_container_updaters.empty()) {
        return;
    }
    auto it = detail::fetch_container_updaters.begin();
    // for (auto it = detail::fetch_container_updaters.begin(); it != detail::fetch_container_updaters.end(); ++it) {
        if (it->second()) { // Check if the condition for this container is satisfied
            detail::fetch_container_updaters.erase(it); // Erase this container from the map
            return; // Exit after processing one container
        }
    // }

}

void wait_fetched_containers()
{
    std::vector<std::uintptr_t> _to_erase = {};
    for (std::pair<const std::uintptr_t, std::function<bool()>>& _pair : detail::fetch_container_updaters) {
        if (_pair.second()) {
            _to_erase.emplace_back(_pair.first);
        }
    }
    for (const std::uintptr_t _element : _to_erase) {
        detail::fetch_container_updaters.erase(_element);
    }
}