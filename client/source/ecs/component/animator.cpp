#include <ecs/component/animator.hpp>
#include <core/fetch.hpp>

animation_controller& animation_controller::play()
{
    _is_playing = true;
    return *this;
}

animation_controller& animation_controller::pause()
{
    _is_playing = false;
    return *this;
}

animation_controller& animation_controller::stop()
{
    _is_playing = false;
    _time_ratio = 0.f;
    return *this;
}

animation_controller& animation_controller::time_warp(const glm::float32 ratio)
{
    _time_ratio = ratio;
    return *this;
}

animation_controller& animation_controller::time_relative(const glm::float32 ratio)
{
    _time_ratio += ratio;
    return *this;
}

animation_controller& animation_controller::loop(const bool enabled)
{
    _is_looping = enabled;
    return *this;
}

animation_controller& animation_controller::speed(const glm::float32 ratio)
{
    _playback_speed = ratio;
    return *this;
}

animator_component& animator_component::animations(const std::unordered_map<glm::uint, std::shared_future<std::shared_ptr<animation_ref>>>& fetched_animations)
{
    for (const std::pair<const glm::uint, std::shared_future<std::shared_ptr<animation_ref>>>& _pair : fetched_animations) {
        const glm::uint _name = _pair.first;
        _animations[_name].emplace(_pair.second, [this, _name] () {
            if (_skeleton.has_value()) {
#if LUCARIA_DEBUG
                const int _animation_tracks = _animations[_name].value().num_tracks();
                const int _skeleton_joints = _skeleton.value().num_joints();
                if (_animation_tracks != _skeleton_joints) {
                    std::cout << "Incompatible animation with " << _animation_tracks << " tracks and skeleton with " << _skeleton_joints << " joints." << std::endl;
                    std::terminate();
                }
#endif
                _local_transforms[_name].resize(_skeleton.value().num_soa_joints());
            }
        });
        _controllers[_name]._local_transforms.resize(_skeleton.value().num_soa_joints());
    }
    return *this;
}
void PrintBoneNames(const ozz::animation::Skeleton& skeleton) {
    const int num_joints = skeleton.num_joints();
    for (int i = 0; i < num_joints; ++i) {
        const char* joint_name = skeleton.joint_names()[i];
        printf("Bone %d: %s\n", i, joint_name);
    }
}


animator_component& animator_component::skeleton(const std::shared_future<std::shared_ptr<skeleton_ref>>& fetched_skeleton)
{
    _skeleton.emplace(fetched_skeleton, [this] () {
        for (const std::pair<const glm::uint, fetch_container<animation_ref>>& _pair : _animations) {
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
        _model_output_transforms.resize(_num_joints);
        _model_transforms.resize(_num_joints, ozz::math::Float4x4::identity());
        _model_last_transforms.resize(_num_joints, ozz::math::Float4x4::identity());
    });
    return *this;
}

animator_component& animator_component::motion_bone(const std::optional<glm::uint>& bone_index)
{
    _motion_bone_index = bone_index;
    return *this;
}

animator_component& animator_component::motion_bone(const std::optional<std::string>& bone_name)
{
    _motion_bone_name = bone_name;
    return *this;
}

animation_controller& animator_component::get_controller(const glm::uint name)
{
    return _controllers.at(_name);
}
