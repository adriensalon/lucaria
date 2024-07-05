#include <ecs/component/animator.hpp>
#include <glue/fetch.hpp>

animator_component& animator_component::animation(const std::string& name, animation_ref&& value)
{
    _animations[name] = std::move(value);
    return *this;
}

animator_component& animator_component::animation(const std::string& name, std::future<animation_ref>&& value)
{
    _future_animations[name] = std::move(value);
    return *this;
}

animator_component& animator_component::skeleton(skeleton_ref&& value)
{
    _skeleton = std::move(value);
    return *this;
}

animator_component& animator_component::skeleton(std::future<skeleton_ref>&& value)
{
    _future_skeleton = std::move(value);
    return *this;
}

void animator_component::_update_futures()
{
    for (std::pair<const std::string, std::optional<std::future<animation_ref>>>& _pair : _future_animations) {
        if (_pair.second.has_value()) {
            std::future<animation_ref>& _future = _pair.second.value();
            if (get_is_future_ready(_future)) {
                _animations[_pair.first] = std::move(_future.get());
                _pair.second = std::nullopt;
            }
        }
    }
    if (_future_skeleton.has_value()) {
        std::future<skeleton_ref>& _future = _future_skeleton.value();
        if (get_is_future_ready(_future)) {
            _skeleton = std::move(_future.get());
            _local_to_model_job.skeleton = &_skeleton.value().get_skeleton();
            _future_skeleton = std::nullopt;
        }
    }
}