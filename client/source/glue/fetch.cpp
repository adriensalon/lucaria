#include <filesystem>
#include <iostream>
#include <vector>
#include <unordered_map>

#include <emscripten/fetch.h>

#include <glue/fetch.hpp>

using fetch_callback = std::function<void(std::istringstream&)>;

namespace detail {

std::unordered_map<std::string, fetch_callback> fetch_requests;

void on_fetch_success(emscripten_fetch_t* fetch)
{
    std::cout << "Successfully fetched " << fetch->numBytes << " bytes from " << fetch->url << std::endl;
    std::vector<char> _data(fetch->data, fetch->data + fetch->numBytes);
    std::istringstream _stream(std::string(_data.begin(), _data.end()), std::ios::binary);
    fetch_requests[fetch->url](_stream);
    emscripten_fetch_close(fetch);
}

void on_fetch_error(emscripten_fetch_t* fetch)
{
    std::cout << "Failed to fetch " << fetch->url << " with status " << fetch->status << std::endl;
    emscripten_fetch_close(fetch);
}

}

void fetch_file(const std::string& url, const fetch_callback& callback, const bool persist)
{
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