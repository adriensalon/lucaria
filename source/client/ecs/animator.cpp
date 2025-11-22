#include <lucaria/core/error.hpp>
#include <lucaria/ecs/animator.hpp>

namespace lucaria {

animation_controller& animation_controller::set_play()
{
    _is_playing = true;
    return *this;
}

animation_controller& animation_controller::set_pause()
{
    _is_playing = false;
    return *this;
}

animation_controller& animation_controller::set_stop()
{
    _is_playing = false;
    _time_ratio = 0.f;
    return *this;
}

animation_controller& animation_controller::set_time(const float ratio)
{
    _time_ratio = ratio;
    return *this;
}

animation_controller& animation_controller::set_loop(const bool enable)
{
    _is_looping = enable;
    return *this;
}

animation_controller& animation_controller::set_speed(const float ratio)
{
    _playback_speed = ratio;
    return *this;
}

animation_controller& animation_controller::set_weight(const float ratio)
{
    _weight = ratio;
    return *this;
}

animation_controller& animation_controller::set_event_callback(const std::string& name, const std::function<void()>& callback)
{
    _event_callbacks[name] = callback;
    return *this;
}

animator_component& animator_component::use_animation(const std::string name, animation& from)
{
    _animations[name].emplace(from);
    if (_skeleton.has_value()) {
#if LUCARIA_CONFIG_DEBUG
        const int _animation_tracks = _animations[name].value().get_handle().num_tracks();
        const int _skeleton_joints = _skeleton.value().get_handle().num_joints();
        if (_animation_tracks != _skeleton_joints) {
            LUCARIA_RUNTIME_ERROR("Incompatible animation with " + std::to_string(_animation_tracks) + " tracks and skeleton with " + std::to_string(_skeleton_joints) + " joints")
        }
#endif
        _local_transforms[name].resize(_skeleton.value().get_handle().num_soa_joints());
    }
    _controllers[name] = animation_controller();
    return *this;
}

animator_component& animator_component::use_animation(const std::string name, fetched<animation>& from)
{
    _animations[name].emplace(from, [this, name]() {
        if (_skeleton.has_value()) {
#if LUCARIA_CONFIG_DEBUG
            const int _animation_tracks = _animations[name].value().get_handle().num_tracks();
            const int _skeleton_joints = _skeleton.value().get_handle().num_joints();
            if (_animation_tracks != _skeleton_joints) {
                LUCARIA_RUNTIME_ERROR("Incompatible animation with " + std::to_string(_animation_tracks) + " tracks and skeleton with " + std::to_string(_skeleton_joints) + " joints")
            }
#endif
            _local_transforms[name].resize(_skeleton.value().get_handle().num_soa_joints());
        }
    });
    _controllers[name] = animation_controller();
    return *this;
}

animator_component& animator_component::use_motion_track(const std::string name, animation_motion_track& from)
{
    if (_animations.find(name) == _animations.end()) {
        LUCARIA_RUNTIME_ERROR("Impossible to emplace motion track because animation does not exist with this name")
    }
    _motion_tracks[name].emplace(from);
    return *this;
}

animator_component& animator_component::use_motion_track(const std::string name, fetched<animation_motion_track>& from)
{
    if (_animations.find(name) == _animations.end()) {
        LUCARIA_RUNTIME_ERROR("Impossible to emplace motion track because animation does not exist with this name")
    }
    _motion_tracks[name].emplace(from);
    return *this;
}

animator_component& animator_component::use_event_track(const std::string name, animation_event_track& from)
{
    if (_animations.find(name) == _animations.end()) {
        LUCARIA_RUNTIME_ERROR("Impossible to emplace event track because animation does not exist with this name")
    }
    _event_tracks[name].emplace(from);
    return *this;
}

animator_component& animator_component::use_event_track(const std::string name, fetched<animation_event_track>& from)
{
    if (_animations.find(name) == _animations.end()) {
        LUCARIA_RUNTIME_ERROR("Impossible to emplace event track because animation does not exist with this name")
    }
    _event_tracks[name].emplace(from);
    return *this;
}

animator_component& animator_component::use_skeleton(skeleton& from)
{
    _skeleton.emplace(from);
    for (const std::pair<const std::string, _detail::fetched_container<animation>>& _pair : _animations) {
        if (_pair.second.has_value()) {
#if LUCARIA_CONFIG_DEBUG
            const int _animation_tracks = _pair.second.value().get_handle().num_tracks();
            const int _skeleton_joints = _skeleton.value().get_handle().num_joints();
            if (_animation_tracks != _skeleton_joints) {
                LUCARIA_RUNTIME_ERROR("Incompatible animation with " + std::to_string(_animation_tracks) + " tracks and skeleton with " + std::to_string(_skeleton_joints) + " joints")
            }
#endif
            _local_transforms[_pair.first].resize(_skeleton.value().get_handle().num_soa_joints());
        }
    }
    const int _num_joints = _skeleton.value().get_handle().num_joints();
    _sampling_context = std::make_unique<ozz::animation::SamplingJob::Context>();
    _sampling_context->Resize(_num_joints);
    _blended_local_transforms.resize(_num_joints);
    _model_transforms.resize(_num_joints, ozz::math::Float4x4::identity());
    return *this;
}

animator_component& animator_component::use_skeleton(fetched<skeleton>& from)
{
    _skeleton.emplace(from, [this]() {
        for (const std::pair<const std::string, _detail::fetched_container<animation>>& _pair : _animations) {
            if (_pair.second.has_value()) {
#if LUCARIA_CONFIG_DEBUG
                const int _animation_tracks = _pair.second.value().get_handle().num_tracks();
                const int _skeleton_joints = _skeleton.value().get_handle().num_joints();
                if (_animation_tracks != _skeleton_joints) {
                    LUCARIA_RUNTIME_ERROR("Incompatible animation with " + std::to_string(_animation_tracks) + " tracks and skeleton with " + std::to_string(_skeleton_joints) + " joints")
                }
#endif
                _local_transforms[_pair.first].resize(_skeleton.value().get_handle().num_soa_joints());
            }
        }
        const int _num_joints = _skeleton.value().get_handle().num_joints();
        _sampling_context = std::make_unique<ozz::animation::SamplingJob::Context>();
        _sampling_context->Resize(_num_joints);
        _blended_local_transforms.resize(_num_joints);
        _model_transforms.resize(_num_joints, ozz::math::Float4x4::identity());
    });

    return *this;
}

animation_controller& animator_component::get_controller(const std::string& name)
{
    return _controllers.at(name);
}

glm::mat4 animator_component::get_bone_transform(const std::string& bone)
{
    if (_skeleton.has_value()) {
        const int _num_joints = _skeleton.value().get_handle().num_joints();
        for (int _joint_index = 0; _joint_index < _num_joints; ++_joint_index) {
            if (std::string(_skeleton.value().get_handle().joint_names()[_joint_index]) == bone) {
                return convert(_model_transforms[_joint_index]);
            }
        }
    }
    return glm::mat4(1);
}

}
