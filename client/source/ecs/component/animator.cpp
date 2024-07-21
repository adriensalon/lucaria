#include <ecs/component/animator.hpp>
#include <core/fetch.hpp>

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
        _controllers[_name] = animation_controller();
    }
    return *this;
}

animator_component& animator_component::armature(const std::shared_future<std::shared_ptr<armature_ref>>& fetched_armature)
{
    _armature.emplace(fetched_armature);
    return *this;
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
        _model_transforms.resize(_num_joints);
    });
    return *this;
}

animator_component& animator_component::motion_bone_index(const std::optional<glm::uint> bone_index)
{
    _motion_bone_index = bone_index;
    return *this;
}