#pragma once

#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/track.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>

#include <lucaria/core/animation.hpp>
#include <lucaria/core/event_track.hpp>
#include <lucaria/core/motion_track.hpp>
#include <lucaria/core/skeleton.hpp>
#include <lucaria/entity/transform.hpp>

namespace lucaria {
namespace detail {
    struct motion_system;
    struct rendering_system;
}

/// @brief
struct animation_controller {
    animation_controller() = default;
    animation_controller(const animation_controller& other) = delete;
    animation_controller& operator=(const animation_controller& other) = delete;
    animation_controller(animation_controller&& other) = default;
    animation_controller& operator=(animation_controller&& other) = default;

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_play();

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_pause();

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_stop();

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_time(const glm::float32 ratio);

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_loop(const bool enable = true);

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_speed(const glm::float32 ratio);

    /// @brief
    /// @return this instance for chaining methods
    animation_controller& set_weight(const glm::float32 ratio);

    /// @brief
    /// @param name
    /// @param callback
    /// @return this instance for chaining methods
    animation_controller& event_callback(const std::string& name, const std::function<void()>& callback);

private:
    bool _is_playing = false;
    bool _is_looping = true;
    glm::float32 _playback_speed = 1.f;
    glm::float32 _weight = 1.f;
    glm::float32 _time_ratio = 0.f;
    glm::float32 _last_time_ratio = 0.f;
    bool _has_looped = false;
    std::unordered_map<std::string, std::function<void()>> _event_callbacks = {};

    template <typename ArchiveType>
    void serialize(ArchiveType& archive)
    {
        archive(cereal::make_nvp("is_playing", _is_playing));
        archive(cereal::make_nvp("is_looping", _is_looping));
        archive(cereal::make_nvp("playback_speed", _playback_speed));
        archive(cereal::make_nvp("weight", _weight));
        archive(cereal::make_nvp("time_ratio", _time_ratio));
        archive(cereal::make_nvp("last_time_ratio", _last_time_ratio));
        archive(cereal::make_nvp("has_looped", _has_looped));
    }

    friend struct detail::motion_system;
    friend struct animator_component;
    friend class cereal::access;
};

/// @brief
struct animator_component {
    animator_component() = default;
    animator_component(const animator_component& other) = delete;
    animator_component& operator=(const animator_component& other) = delete;
    animator_component(animator_component&& other) = default;
    animator_component& operator=(animator_component&& other) = default;

    /// @brief
    /// @param name
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_animation(const std::string name, const animation_object animation);

    /// @brief
    /// @param name
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_motion_track(const std::string name, const motion_track_object track);

    /// @brief
    /// @param name
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_event_track(const std::string name, const event_track_object track);

    /// @brief
    /// @param from
    /// @return this instance for chaining methods
    animator_component& use_skeleton(const skeleton_object skeleton);

    // animator_component& use_inverse_kinematics_chain(const std::string name, const std::string& start, const std::string& end);
    // animator_component& use_inverse_kinematics_snap(const std::string name, const glm::vec3& end_position);

    /// @brief
    /// @param name
    /// @return
    [[nodiscard]] animation_controller& get_controller(const std::string& name);

    /// @brief
    /// @param bone
    /// @return
    [[nodiscard]] glm::mat4 get_bone_transform(const std::string& bone);

private:
    skeleton_object _skeleton = {};
    std::unordered_map<std::string, animation_object> _animations = {};
    std::unordered_map<std::string, motion_track_object> _motion_tracks = {};
    std::unordered_map<std::string, event_track_object> _event_tracks = {};
    std::unordered_map<std::string, std::vector<std::reference_wrapper<transform_component>>> _children_transforms = {};
    std::unordered_map<std::string, animation_controller> _controllers = {};
    std::unordered_map<std::string, ozz::vector<ozz::math::SoaTransform>> _local_transforms = {};
    ozz::vector<ozz::math::SoaTransform> _blended_local_transforms = {};
    ozz::vector<ozz::math::Float4x4> _model_transforms = {};
    std::unique_ptr<ozz::animation::SamplingJob::Context> _sampling_context = nullptr;
    ozz::vector<ozz::animation::BlendingJob::Layer> _blend_layers = {};

    template <typename ArchiveType>
    void save(ArchiveType& archive) const
    {
        archive(cereal::make_nvp("skeleton", _skeleton));
        archive(cereal::make_nvp("animations", _animations));
        archive(cereal::make_nvp("motion_tracks", _motion_tracks));
        archive(cereal::make_nvp("event_tracks", _event_tracks));
        archive(cereal::make_nvp("controllers", _controllers));
    }

    template <typename ArchiveType>
    void load(ArchiveType& archive)
    {
        archive(cereal::make_nvp("skeleton", _skeleton));
        archive(cereal::make_nvp("animations", _animations));
        archive(cereal::make_nvp("motion_tracks", _motion_tracks));
        archive(cereal::make_nvp("event_tracks", _event_tracks));
        archive(cereal::make_nvp("controllers", _controllers));
        use_skeleton(_skeleton);
        for (auto& [name, animation] : _animations) {
            use_animation(name, animation);
        }
        for (auto& [name, track] : _motion_tracks) {
            use_motion_track(name, track);
        }
        for (auto& [name, track] : _event_tracks) {
            use_event_track(name, track);
        }
    }

    friend struct detail::motion_system;
    friend struct detail::rendering_system;
    friend struct transform_component;
    friend class cereal::access;
};

}
