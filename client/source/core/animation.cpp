#include <iostream>

#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <core/animation.hpp>
#include <core/fetch.hpp>
#include <core/load.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<animation_ref>>> promises;

}

std::shared_future<std::shared_ptr<animation_ref>> fetch_animation(const std::filesystem::path& animation_path)
{
    std::promise<std::shared_ptr<animation_ref>>& _promise = detail::promises[animation_path.string()];
//     fetch_file(animation_path, [&_promise](std::istringstream& stream) {
//         std::shared_ptr<animation_ref> _animation = std::make_shared<animation_ref>();
//         ozz::io::StdStringStreamWrapper _ozz_stream(stream);
//         {            
//             ozz::io::IArchive _ozz_archive(&_ozz_stream);
// #if LUCARIA_DEBUG
//             if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
//                 std::cout << "Impossible to load animation, archive doesn't contain the expected object type." << std::endl;
//                 std::terminate();
//             }
// #endif
//             _ozz_archive >> *(_animation.get());
// #if LUCARIA_DEBUG
//             std::cout << "Loaded animation with " << _animation->num_tracks() << " tracks." << std::endl;
// #endif
//         }
//         _promise.set_value(_animation);
//     });
    fetch_file(animation_path, [&_promise](const std::vector<char>& stream) {
        std::shared_ptr<animation_ref> _animation = std::make_shared<animation_ref>();
        ozz_raw_input_stream _ozz_stream(stream);
        {            
            ozz::io::IArchive _ozz_archive(&_ozz_stream);
#if LUCARIA_DEBUG
            if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
                std::cout << "Impossible to load animation, archive doesn't contain the expected object type." << std::endl;
                std::terminate();
            }
#endif
            _ozz_archive >> *(_animation.get());
#if LUCARIA_DEBUG
            std::cout << "Loaded animation with " << _animation->num_tracks() << " tracks." << std::endl;
#endif
        }
        _promise.set_value(_animation);
    });
    return _promise.get_future();
}
