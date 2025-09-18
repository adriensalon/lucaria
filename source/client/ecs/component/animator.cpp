#include <lucaria/ecs/component/animator.hpp>
#include <lucaria/core/fetch.hpp>

namespace detail {

    
void PrintBoneNames(const ozz::animation::Skeleton& skeleton) {
    const int num_joints = skeleton.num_joints();
    for (int i = 0; i < num_joints; ++i) {
        const char* joint_name = skeleton.joint_names()[i];
        printf("Bone %d: %s\n", i, joint_name);
    }
}

}

animation_controller& animation_controller::play()
{
    _is_playing = true;
    return *this;
}

animation_controller& animation_controller::pause()
{
    _is_playing = false;
    _last_time_ratio = _time_ratio;
    return *this;
}

animation_controller& animation_controller::stop()
{
    _is_playing = false;
    _last_time_ratio = _time_ratio;
    return *this;
}

animation_controller& animation_controller::time(const float ratio)
{
    _time_ratio = ratio;
    return *this;
}

animation_controller& animation_controller::loop(const bool enabled)
{
    _is_looping = enabled;
    return *this;
}

animation_controller& animation_controller::speed(const float ratio)
{
    _playback_speed = ratio;
    return *this;
}

// animation_controller& fade_in(const float duration = 0.1f); // trigger once then disappear
// animation_controller& fade_out(const float duration = 0.1f); // trigger once then disappear
// animation_controller& weight(const float ratio);

animator_component& animator_component::animation(const unsigned int name, const std::shared_future<std::shared_ptr<animation_ref>>& fetched_animation)
{
    _animations[name].emplace(fetched_animation, [this, name] () {
        if (_skeleton.has_value()) {
#if LUCARIA_DEBUG
            const int _animation_tracks = _animations[name].value().num_tracks();
            const int _skeleton_joints = _skeleton.value().num_joints();
            if (_animation_tracks != _skeleton_joints) {
                std::cout << "Incompatible animation with " << _animation_tracks << " tracks and skeleton with " << _skeleton_joints << " joints." << std::endl;
                std::terminate();
            }
#endif
            _local_transforms[name].resize(_skeleton.value().num_soa_joints());
        }
    });
    _controllers[name] = animation_controller();
    return *this;
}

animator_component& animator_component::motion_track(const unsigned int name, const std::shared_future<std::shared_ptr<motion_track_ref>>& fetched_motion_track)
{
#if LUCARIA_DEBUG
    if (_animations.find(name) == _animations.end()) {
        std::cout << "Impossible to emplace motion track because animation does not exist with this name." << std::endl;
        std::terminate();
    }
#endif
    _motion_tracks[name].emplace(fetched_motion_track);
    return *this;
}

animator_component& animator_component::skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton)
{
    _skeleton.emplace(fetched_skeleton, [this] () {
        for (const std::pair<const unsigned int, fetch_container<animation_ref>>& _pair : _animations) {
            if (_pair.second.has_value()) {
#if LUCARIA_DEBUG
                const int _animation_tracks = _pair.second.value().num_tracks();
                const int _skeleton_joints = _skeleton.value().num_joints();
                if (_animation_tracks != _skeleton_joints) {
                    std::cout << "Incompatible animation with " << _animation_tracks << " tracks and skeleton with " << _skeleton_joints << " joints." << std::endl;
                    std::terminate();
                }
#endif
                _local_transforms[_pair.first].resize(_skeleton.value().num_soa_joints());
            }
        }
        const int _num_joints = _skeleton.value().num_joints();
        _sampling_context = std::make_unique<ozz::animation::SamplingJob::Context>();
        _sampling_context->Resize(_num_joints);
        _blended_local_transforms.resize(_num_joints);
        _model_transforms.resize(_num_joints, ozz::math::Float4x4::identity());
    });
    return *this;
}

animation_controller& animator_component::get_controller(const unsigned int name)
{
    return _controllers.at(name);
}
