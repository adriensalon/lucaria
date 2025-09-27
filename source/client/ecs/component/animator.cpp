#include <string>

#include <lucaria/core/error.hpp>
#include <lucaria/ecs/component/animator.hpp>

namespace lucaria {
namespace ecs {
    namespace {

        void PrintBoneNames(const ozz::animation::Skeleton& skeleton)
        {
            const int num_joints = skeleton.num_joints();
            for (int i = 0; i < num_joints; ++i) {
                const char* joint_name = skeleton.joint_names()[i];
                printf("Bone %d: %s\n", i, joint_name);
            }
        }

    }

    // controller

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

    // animator

    animator_component& animator_component::use_animation(const std::string name, animation& from)
    {
        _animations[name].emplace(from);

        if (_skeleton.has_value()) {
#if LUCARIA_DEBUG
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
#if LUCARIA_DEBUG
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

    animator_component& animator_component::use_motion_track(const std::string name, motion_track& from)
    {
        if (_animations.find(name) == _animations.end()) {
            LUCARIA_RUNTIME_ERROR("Impossible to emplace motion track because animation does not exist with this name")
        }

        _motion_tracks[name].emplace(from);
        return *this;
    }

    animator_component& animator_component::use_motion_track(const std::string name, fetched<motion_track>& from)
    {
        if (_animations.find(name) == _animations.end()) {
            LUCARIA_RUNTIME_ERROR("Impossible to emplace motion track because animation does not exist with this name")
        }

        _motion_tracks[name].emplace(from);
        return *this;
    }

    animator_component& animator_component::use_skeleton(skeleton& from)
    {
        _skeleton.emplace(from);

        for (const std::pair<const std::string, detail::fetched_container<animation>>& _pair : _animations) {
            if (_pair.second.has_value()) {
#if LUCARIA_DEBUG
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

            for (const std::pair<const std::string, detail::fetched_container<animation>>& _pair : _animations) {
                if (_pair.second.has_value()) {
#if LUCARIA_DEBUG
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

            const int num_joints = _skeleton.value().get_handle().num_joints();
            for (int i = 0; i < num_joints; ++i) {
                if (std::string(_skeleton.value().get_handle().joint_names()[i]) == bone) {
                    return detail::reinterpret(_model_transforms[i]);
                }
            }
        }
        
        return glm::mat4(1.f);
    }

}
}
