#include <ecs/component/animator.hpp>
#include <glue/fetch.hpp>

animation_sampler_ref::animation_sampler_ref()
{
    is_prepared = false;
    is_playing = false;
    is_looping = false;
    ratio = 0.5f;
    weight = 1.f;
}

animator_component::animator_component()
{
    _is_prepared = false;
    _is_bound_to_model = false;
}

animator_component& animator_component::animation(const std::string& name, animation_ref&& value)
{
    _animations.insert_or_assign(name, std::move(value));
    _samplers.insert_or_assign(name, animation_sampler_ref());
    return *this;
}

animator_component& animator_component::animation(const std::string& name, std::future<animation_ref>&& value)
{
    _future_animations.insert_or_assign(name, std::move(value));
    _animations.insert_or_assign(name, std::nullopt);
    _samplers.insert_or_assign(name, animation_sampler_ref());
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

animator_component& animator_component::play(const std::string& value)
{
    _samplers.at(value).is_playing = true;
    return *this;
}

void animator_component::test()
{
    


}