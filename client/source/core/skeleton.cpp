#include <iostream>

#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <core/skeleton.hpp>
#include <glue/fetch.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<skeleton_ref>>> promises;

}

skeleton_ref::skeleton_ref(const skeleton_data& data)
{
    _skeleton = std::move(*(data.get()));
}

ozz::animation::LocalToModelJob& skeleton_ref::get_job()
{
    return _local_to_model_job;
}

glm::uint skeleton_ref::get_transforms_size() const
{
    return static_cast<glm::uint>(_skeleton.num_joints());
}

glm::uint skeleton_ref::get_soa_transforms_size() const
{
    return static_cast<glm::uint>(_skeleton.num_soa_joints());
}

skeleton_data load_skeleton_data(std::istringstream& skeleton_stream)
{
    skeleton_data _data = std::make_shared<ozz::animation::Skeleton>();
    {
        ozz::io::StdStringStreamWrapper _ozz_stream(skeleton_stream);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
#if LUCARIA_DEBUG
        if (!_ozz_archive.TestTag<ozz::animation::Skeleton>()) {
            std::cout << "Impossible to load skeleton, archive doesn't contain the expected object type." << std::endl;
            std::terminate();
        }
#endif
        _ozz_archive >> *(_data.get());
    }
    return _data;
}

std::shared_future<std::shared_ptr<skeleton_ref>> fetch_skeleton(const std::filesystem::path& skeleton_path)
{
    std::promise<std::shared_ptr<skeleton_ref>>& _promise = detail::promises[skeleton_path.string()];
    fetch_file(skeleton_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<skeleton_ref>(load_skeleton_data(stream))));
    });
    return _promise.get_future();
}
