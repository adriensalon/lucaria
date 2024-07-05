#include <iostream>

#include <ozz/animation/runtime/animation.h>
#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <glue/fetch.hpp>
#include <core/animation.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<animation_ref>> promises;

}


animation_ref::animation_ref(animation_ref&& other)
{

}

animation_ref& animation_ref::operator=(animation_ref&& other)
{
    return *this;
}

animation_ref::~animation_ref()
{

}


// animation_ref load_animation(const std::filesystem::path& file)
// {

// }

std::future<animation_ref> fetch_animation(const std::filesystem::path& file)
{
    const std::string _file_str = file.string();
    std::promise<animation_ref>& _promise = detail::promises[_file_str];
    fetch_file(_file_str, [&_promise, file](std::istringstream& stream) {
        ozz_istringstream _ozz_stream(stream);
        ozz::io::IArchive archive(&_ozz_stream);

        if (!archive.TestTag<ozz::animation::Animation>()) {
            std::cout << "Archive doesn't contain the expected object type." << std::endl;
        } else {
            std::cout << "Archive contains animations !" << std::endl;
        }
        
        ozz::animation::Animation animation;
        archive >> animation;
        
// #if LUCARIA_DEBUG
//         std::cout << "Loaded animation data from " << file << " ("
//                   << _data.count << " vertices)" << std::endl;
// #endif
//         _promise.set_value(std::move(mesh_ref(_data)));
    });
    return _promise.get_future();
}
