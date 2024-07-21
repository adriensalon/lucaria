#include <iostream>

#include <btBulletDynamicsCommon.h>
#include <ozz/base/maths/soa_transform.h>

#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/system/motion.hpp>
#include <core/world.hpp>

namespace detail {

#if LUCARIA_GUIZMO
extern void draw_guizmo_line(const btVector3& from, const btVector3& to, const btVector3& color);

void draw_guizmo_cone(const btVector3& from, const btVector3& to, const btVector3& color) {
    
    // Draw the cone (you need to implement this based on your rendering framework)
    // This could involve setting up a vertex buffer for a cone mesh and rendering it with the calculated transform
    // For simplicity, we'll just print out the cone parameters here
    std::cout << "Draw cone from (" << from.x() << ", " << from.y() << ", " << from.z() << ") "
              << "to (" << to.x() << ", " << to.y() << ", " << to.z() << ") "
              << "with color (" << color.x() << ", " << color.y() << ", " << color.z() << ")\n";
}

#endif

}

void motion_system::blend_animations()
{
    // animators -> model space matrices w extracted root motion

    // sampling job
    each_level([](entt::registry& registry) {
        registry.view<animator_component>().each([](animator_component& animator) {
            for (const std::pair<const glm::uint, fetch_container<animation_ref>>& _pair : animator._animations) {
                if (_pair.second.has_value()) {
                    // _pair.second.value().get_animation().
                }
            }
            
        });
    });
}

void motion_system::apply_root_motion()
{
    // animators, transforms
}

void motion_system::apply_foot_ik()
{
    // animators, rigidbodies (stores ground_normal)
}

void motion_system::skin_meshes()
{
    // animators, models
}

void motion_system::collect_debug_guizmos()
{
#if LUCARIA_GUIZMO
    each_level([](entt::registry& registry) {
        registry.view<animator_component>().each([](animator_component& animator) {
            if (animator._skeleton.has_value()) {
                // add cones from animator._model_transforms with detail::draw_line(from, to, color)

                skeleton_ref& skeleton = animator._skeleton.value();
                const ozz::vector<ozz::math::Float4x4>& model_transforms = animator._model_transforms;
                // Ensure the skeleton and model transforms are valid
                if (!model_transforms.empty()) {
                    const auto& joint_parents = skeleton.joint_parents();

                    // Iterate through each joint to draw cones
                    for (size_t i = 0; i < model_transforms.size(); ++i) {
                        int parent_index = joint_parents[i];
                        if (parent_index == ozz::animation::Skeleton::kNoParent) {
                            continue;  // Skip root joints
                        }

                        // Get the current and parent joint transforms
                        const auto& current_transform = model_transforms[i];
                        const auto& parent_transform = model_transforms[parent_index];

                        // Convert Ozz transforms to Bullet vectors
                        btVector3 from(parent_transform.cols[3].x, parent_transform.cols[3].y, parent_transform.cols[3].z);
                        btVector3 to(current_transform.cols[3].x, current_transform.cols[3].y, current_transform.cols[3].z);
                        btVector3 color(1.0f, 0.0f, 0.0f);  // Red color for bones

                        // Draw the cone
                        detail::draw_guizmo_cone(from, to, color);
                    }
                }
            }
        });
    });
#endif
}