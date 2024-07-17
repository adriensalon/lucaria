#include <iostream>

#include <ozz/base/io/archive.h>
#include <ozz/base/memory/allocator.h>

#include <core/animation.hpp>
#include <core/fetch.hpp>

namespace detail {

static std::unordered_map<std::string, std::promise<std::shared_ptr<animation_ref>>> promises;

}

animation_ref::animation_ref(const animation_data& data)
{
    _animation = std::move(*(data.get()));
    _sampling_job.animation = &_animation;
    _sampling_job.context->Resize(_animation.num_tracks());
    // _sampling_job.output = ozz::make_span(_local_transforms);
    _sampling_job.ratio = 0.f;
    // _sampling_job.Validate();
    // bind skeleton? for num joints
    // animation_sampler.local_transforms.resize(skeleton.get_skeleton().num_soa_joints());
}

float animation_ref::get_duration() const
{
    return _animation.duration();
}

ozz::animation::SamplingJob& animation_ref::get_job()
{
    return _sampling_job;
}

animation_data load_animation_data(std::istringstream& stream)
{
    animation_data _data = std::make_shared<ozz::animation::Animation>();
    {
        ozz::io::StdStringStreamWrapper _ozz_stream(stream);
        ozz::io::IArchive _ozz_archive(&_ozz_stream);
#if LUCARIA_DEBUG
        if (!_ozz_archive.TestTag<ozz::animation::Animation>()) {
            std::cout << "Impossible to load animation, archive doesn't contain the expected object type." << std::endl;
            std::terminate();
        }
#endif
        _ozz_archive >> *(_data.get());
    }
    return _data;
}

std::shared_future<std::shared_ptr<animation_ref>> fetch_animation(const std::filesystem::path& animation_path)
{
    std::promise<std::shared_ptr<animation_ref>>& _promise = detail::promises[animation_path.string()];
    fetch_file(animation_path, [&_promise](std::istringstream& stream) {
        _promise.set_value(std::move(std::make_shared<animation_ref>(load_animation_data(stream))));
    });
    return _promise.get_future();
}
