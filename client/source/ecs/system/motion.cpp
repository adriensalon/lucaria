#include <iostream>
#include <random>

#include <btBulletDynamicsCommon.h>
#include <glm/gtc/type_ptr.hpp>
#include <ozz/animation/runtime/local_to_model_job.h>


#include <core/window.hpp>
#include <core/world.hpp>
#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/component/transform.hpp>
#include <ecs/system/motion.hpp>

btVector3 get_random_color() {
    // Create a random number generator with a uniform distribution between 0.0 and 1.0
    static std::random_device rd; // Obtain a random number from hardware
    static std::mt19937 generator(rd()); // Seed the generator
    static std::uniform_real_distribution<glm::float32> distribution(0.0f, 1.0f); // Define the range

    // Generate random colors
    return btVector3(distribution(generator), distribution(generator), distribution(generator));
}




namespace detail {

#if LUCARIA_GUIZMO
extern void draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);

void draw_guizmo_cone(const btVector3& from, const btVector3& to, const btVector3& color)
{
    draw_guizmo_line(from, to, color);
}

#endif

glm::mat4 ozz_to_glm(const ozz::math::Float4x4& matrix)
{
    glm::mat4 _matrix;
    std::memcpy(glm::value_ptr(_matrix), matrix.cols, sizeof(matrix.cols));
    return _matrix;
}

}

void print_matrix(const ozz::math::Float4x4& matrix)
{
    // const ozz::math::SimdFloat4 cols[] = matrix.cols;

    // Print the matrix in a 4x4 grid
    std::cout << std::endl;
    std::cout << "[ " << matrix.cols[0].x << " " << matrix.cols[1].x << " " << matrix.cols[2].x << " " << matrix.cols[3].x << " ]" << std::endl;
    std::cout << "[ " << matrix.cols[0].y << " " << matrix.cols[1].y << " " << matrix.cols[2].y << " " << matrix.cols[3].y << " ]" << std::endl;
    std::cout << "[ " << matrix.cols[0].z << " " << matrix.cols[1].z << " " << matrix.cols[2].z << " " << matrix.cols[3].z << " ]" << std::endl;
    std::cout << "[ " << matrix.cols[0].w << " " << matrix.cols[1].w << " " << matrix.cols[2].w << " " << matrix.cols[3].w << " ]" << std::endl;
}

void motion_system::blend_animations()
{
    glm::float32 _time_delta = get_time_delta();
    each_level([](entt::registry& registry) {
        registry.view<animator_component>().each([](animator_component& animator) {
            if (animator._skeleton.has_value()) {
                ozz::animation::Skeleton& _skeleton = animator._skeleton.value();
                for (std::pair<const glm::uint, fetch_container<animation_ref>>& _pair : animator._animations) {
                    if (_pair.second.has_value()) {
                        animation_controller& _controller = animator._controllers[_pair.first];
                        ozz::animation::Animation& _animation = _pair.second.value();
                        ozz::vector<ozz::math::SoaTransform>& _local_transforms = animator._local_transforms[_pair.first];

                        // update controller with delta_time

                        if (get_is_audio_locked() && (get_fetches_completed() == get_fetches_total()))
                            _controller._time_ratio += 0.01f;
                        if (_controller._time_ratio > 1.f) {
                            _controller._has_looped = true;
                        }
                        _controller._time_ratio = glm::mod(_controller._time_ratio, 1.f);

                        ozz::animation::SamplingJob sampling_job;
                        sampling_job.animation = &_animation;
                        sampling_job.context = animator._sampling_context.get();
                        sampling_job.ratio = _controller._time_ratio;
                        sampling_job.output = make_span(_local_transforms);
                        if (!sampling_job.Run()) {
#if LUCARIA_DEBUG
                            std::cout << "Impossible to run sampling job." << std::endl;
                            std::terminate();
#endif
                        }

                        // ca faudra bouger
                        ozz::animation::LocalToModelJob ltm_job;
                        ltm_job.skeleton = &_skeleton;
                        ltm_job.input = make_span(_local_transforms);
                        ltm_job.output = make_span(animator._model_transforms);
                        if (!ltm_job.Run()) {
#if LUCARIA_DEBUG
                            std::cout << "Impossible to run local to model job." << std::endl;
                            std::terminate();
#endif
                        }
                    }
                }
            }
        });
    });
}

void motion_system::apply_root_motion()
{
    each_level([](entt::registry& registry) {
        registry.view<animator_component, transform_component>().each([](animator_component& animator, transform_component& transform) {
            
            // todo
        });
    });
}

void motion_system::collect_debug_guizmos()
{
#if LUCARIA_GUIZMO
    each_level([](entt::registry& registry) {
        registry.view<animator_component>(entt::exclude<transform_component>).each([](animator_component& animator) {
            if (animator._skeleton.has_value()) {
                const ozz::vector<ozz::math::Float4x4>& model_transforms = animator._model_transforms;
                if (!model_transforms.empty()) {
                    const auto& joint_parents = animator._skeleton.value().joint_parents();
#if LUCARIA_DEBUG
                    if (model_transforms.size() != joint_parents.size()) {
                        std::cout << "Mismatch between model transforms and joint parents sizes." << std::endl;
                        std::terminate();
                    }
#endif
                    for (size_t i = 0; i < model_transforms.size(); ++i) {
                        int parent_index = joint_parents[i];
                        if (parent_index == ozz::animation::Skeleton::kNoParent) {
                            // // Handle root bone separately
                            continue;
                        }
                        const ozz::math::Float4x4& current_transform = model_transforms[i];
                        const ozz::math::Float4x4& parent_transform = model_transforms[parent_index];
                        btVector3 from(parent_transform.cols[3].x, parent_transform.cols[3].y, parent_transform.cols[3].z);
                        btVector3 to(current_transform.cols[3].x, current_transform.cols[3].y, current_transform.cols[3].z);
                        btVector3 color(1.0f, 0.0f, 0.0f); // Red color for bones
                        detail::draw_guizmo_cone(from, to, color);
                    }
                }
            }
        });
        registry.view<animator_component, transform_component>().each([](animator_component& animator, transform_component& transform) {
            if (animator._skeleton.has_value()) {
                const ozz::vector<ozz::math::Float4x4>& model_transforms = animator._model_transforms;
                if (!model_transforms.empty()) {
                    const auto& joint_parents = animator._skeleton.value().joint_parents();
#if LUCARIA_DEBUG
                    if (model_transforms.size() != joint_parents.size()) {
                        std::cout << "Mismatch between model transforms and joint parents sizes." << std::endl;
                        std::terminate();
                    }
#endif
                    for (size_t i = 0; i < model_transforms.size(); ++i) {
                        int parent_index = joint_parents[i];
                        if (parent_index == ozz::animation::Skeleton::kNoParent) {
                            // // Handle root bone separately
                            continue;
                        }
                        const ozz::math::Float4x4 _modifier_transform = *(reinterpret_cast<ozz::math::Float4x4*>(&transform._transform));
                        const ozz::math::Float4x4 current_transform = _modifier_transform * model_transforms[i];
                        const ozz::math::Float4x4 parent_transform = _modifier_transform * model_transforms[parent_index];
                        btVector3 from(parent_transform.cols[3].x, parent_transform.cols[3].y, parent_transform.cols[3].z);
                        btVector3 to(current_transform.cols[3].x, current_transform.cols[3].y, current_transform.cols[3].z);
                        btVector3 color(1.0f, 0.0f, 0.0f); // Red color for bones
                        detail::draw_guizmo_cone(from, to, color);
                    }
                }
            }
        });
    });
#endif
}