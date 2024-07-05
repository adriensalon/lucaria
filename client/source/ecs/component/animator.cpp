#include <ecs/component/animator.hpp>
#include <glue/fetch.hpp>

animator_component& animator_component::animation(const std::string& name, animation_ref&& value)
{
    _animations.insert_or_assign(name, std::move(value));
    _samplers.insert_or_assign(name, _animation_sampler());
    return *this;
}

animator_component& animator_component::animation(const std::string& name, std::future<animation_ref>&& value)
{
    _future_animations.insert_or_assign(name, std::move(value));
    _animations.insert_or_assign(name, std::nullopt);
    _samplers.insert_or_assign(name, _animation_sampler());
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
    _skeleton = std::nullopt;
    return *this;
}