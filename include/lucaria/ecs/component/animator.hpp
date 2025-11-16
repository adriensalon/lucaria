#pragma once

#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/track.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>

#include <lucaria/core/animation.hpp>
#include <lucaria/core/fetch.hpp>
#include <lucaria/core/math.hpp>
#include <lucaria/core/skeleton.hpp>
#include <lucaria/core/track.hpp>
#include <lucaria/ecs/component/transform.hpp>

namespace lucaria {
namespace detail {
    struct motion_system;
    struct rendering_system;
}

namespace ecs {

    struct animation_controller {
        animation_controller() = default;
        animation_controller(const animation_controller& other) = delete;
        animation_controller& operator=(const animation_controller& other) = delete;
        animation_controller(animation_controller&& other) = default;
        animation_controller& operator=(animation_controller&& other) = default;

        animation_controller& play();
        animation_controller& pause();
        animation_controller& stop();
        animation_controller& time(const float ratio);
        animation_controller& loop(const bool enabled = true);
        animation_controller& speed(const float ratio);
        animation_controller& fade_in(const float duration = 0.1f); // trigger once then disappear
        animation_controller& fade_out(const float duration = 0.1f); // trigger once then disappear
        animation_controller& weight(const float ratio);

        animation_controller& set_on_event(const std::string& name, const std::function<void()>& callback);

    private:
        bool _is_playing = false;
        bool _is_looping = true;
        float _playback_speed = 1.f;
        float _weight = 1.f;
        float _computed_weight = 1.f;
        float _time_ratio = 0.f;
        std::optional<std::pair<float, float>> _fade_in_time_and_duration = std::nullopt;
        std::optional<std::pair<float, float>> _fade_out_time_and_duration = std::nullopt;
        float _last_time_ratio = 0.f;
        bool _has_looped = false;
        std::unordered_map<std::string, std::function<void()>> _event_callbacks = {};
        friend struct detail::motion_system;
        friend struct animator_component;
    };

    struct animator_component {
        animator_component() = default;
        animator_component(const animator_component& other) = delete;
        animator_component& operator=(const animator_component& other) = delete;
        animator_component(animator_component&& other) = default;
        animator_component& operator=(animator_component&& other) = default;

        animator_component& use_animation(const std::string name, animation& from);
        animator_component& use_animation(const std::string name, fetched<animation>& from);
        animator_component& use_motion_track(const std::string name, motion_track& from);
        animator_component& use_motion_track(const std::string name, fetched<motion_track>& from);
        animator_component& use_event_track(const std::string name, event_track& from);
        animator_component& use_event_track(const std::string name, fetched<event_track>& from);
        animator_component& use_skeleton(skeleton& from);
        animator_component& use_skeleton(fetched<skeleton>& from);
        // animator_component& use_inverse_kinematics_chain(const std::string name, const std::string& start, const std::string& end);
        // animator_component& use_inverse_kinematics_snap(const std::string name, const glm::vec3& end_position);

        [[nodiscard]] animation_controller& get_controller(const std::string& name);
        [[nodiscard]] glm::mat4 get_bone_transform(const std::string& bone);
        [[nodiscard]] glm::vec3 get_translation_speed(const std::string& name);

    private:
        detail::fetched_container<skeleton> _skeleton = {};
        std::unordered_map<std::string, detail::fetched_container<animation>> _animations = {};
        std::unordered_map<std::string, detail::fetched_container<motion_track>> _motion_tracks = {};
        std::unordered_map<std::string, detail::fetched_container<event_track>> _event_tracks = {};
        ozz::vector<ozz::math::SoaTransform> _blended_local_transforms = {};
        ozz::vector<ozz::math::Float4x4> _model_transforms = {};
        std::unordered_map<std::string, std::vector<std::reference_wrapper<transform_component>>> _children_transforms = {};
        std::unique_ptr<ozz::animation::SamplingJob::Context> _sampling_context = nullptr;
        std::unordered_map<std::string, animation_controller> _controllers = {};
        std::unordered_map<std::string, ozz::vector<ozz::math::SoaTransform>> _local_transforms = {};
        ozz::vector<ozz::animation::BlendingJob::Layer> _blend_layers = {};
        friend struct transform_component;
        friend struct detail::motion_system;
        friend struct detail::rendering_system;
    };

}
}
