#include <lucaria/core/utils_math.hpp>
#include <lucaria/public/component_animator.hpp>

namespace lucaria {

component_animator_controller& component_animator_controller::set_play()
{
    _is_playing = true;
    return *this;
}

component_animator_controller& component_animator_controller::set_pause()
{
    _is_playing = false;
    return *this;
}

component_animator_controller& component_animator_controller::set_stop()
{
    _is_playing = false;
    _time_ratio = 0.f;
    return *this;
}

component_animator_controller& component_animator_controller::set_time(const float ratio)
{
    _time_ratio = ratio;
    return *this;
}

component_animator_controller& component_animator_controller::set_loop(const bool enable)
{
    _is_looping = enable;
    return *this;
}

component_animator_controller& component_animator_controller::set_speed(const float ratio)
{
    _playback_speed = ratio;
    return *this;
}

component_animator_controller& component_animator_controller::set_weight(const float ratio)
{
    _weight = ratio;
    return *this;
}

component_animator_controller& component_animator_controller::event_callback(const std::string& name, const std::function<void()>& callback)
{
    _event_callbacks[name] = callback;
    return *this;
}

component_animator& component_animator::use_animation(const std::string name, const handle_animation animation)
{
    _animations.insert_or_assign(name, animation);
    _animations.at(name)._cached->fetched.on_ready([this, name]() {
        if (_skeleton) {
#if defined(LUCARIA_DEBUG)
            const int _animation_tracks = _animations[name]._cached->fetched.value().animation.num_tracks();
            const int _skeleton_joints = _skeleton._cached->fetched.value().skeleton.num_joints();
            if (_animation_tracks != _skeleton_joints) {
                LUCARIA_DEBUG_ERROR("Incompatible animation with " + std::to_string(_animation_tracks) + " tracks and skeleton with " + std::to_string(_skeleton_joints) + " joints")
            }
#endif
            _local_transforms[name].resize(_skeleton._cached->fetched.value().skeleton.num_soa_joints());
        }
    });
    _controllers[name] = component_animator_controller();
    return *this;
}

component_animator& component_animator::use_motion_track(const std::string name, const handle_motion_track track)
{
    if (_animations.find(name) == _animations.end()) {
        LUCARIA_DEBUG_ERROR("Impossible to emplace motion track because animation does not exist with this name")
    }
    _motion_tracks.insert_or_assign(name, track);
    return *this;
}

component_animator& component_animator::use_event_track(const std::string name, const handle_event_track track)
{
    if (_animations.find(name) == _animations.end()) {
        LUCARIA_DEBUG_ERROR("Impossible to emplace event track because animation does not exist with this name")
    }
    _event_tracks.insert_or_assign(name, track);
    return *this;
}

component_animator& component_animator::use_skeleton(const handle_skeleton skeleton)
{
    _skeleton = skeleton;
    _skeleton._cached->fetched.on_ready([this]() {
        const int _num_joints = _skeleton._cached->fetched.value().skeleton.num_joints();
        const int _num_soa_joints = _skeleton._cached->fetched.value().skeleton.num_soa_joints();

        for (const std::pair<const std::string, handle_animation>& _pair : _animations) {
            if (_pair.second) {
#if defined(LUCARIA_DEBUG)
                const int _animation_tracks = _pair.second._cached->fetched.value().animation.num_tracks();
                const int _skeleton_joints = _skeleton._cached->fetched.value().skeleton.num_joints();
                if (_animation_tracks != _skeleton_joints) {
                    LUCARIA_DEBUG_ERROR("Incompatible animation with " + std::to_string(_animation_tracks) + " tracks and skeleton with " + std::to_string(_skeleton_joints) + " joints")
                }
#endif
                _local_transforms[_pair.first].resize(_num_soa_joints);
            }
        }
        _sampling_context = std::make_unique<ozz::animation::SamplingJob::Context>();
        _sampling_context->Resize(_num_joints);
        _blended_local_transforms.resize(_num_soa_joints);
        _model_transforms.resize(_num_joints, ozz::math::Float4x4::identity());
    });
    return *this;
}

component_animator_controller& component_animator::get_controller(const std::string& name)
{
    return _controllers.at(name);
}

glm::mat4 component_animator::get_bone_transform(const std::string& bone)
{
    if (_skeleton) {
        const int _num_joints = _skeleton._cached->fetched.value().skeleton.num_joints();
        for (int _joint_index = 0; _joint_index < _num_joints; ++_joint_index) {
            if (std::string(_skeleton._cached->fetched.value().skeleton.joint_names()[_joint_index]) == bone) {
                return detail::convert(_model_transforms[_joint_index]);
            }
        }
    }
    return glm::mat4(1);
}

}
