
#include <ozz/base/maths/soa_transform.h>

#include <ecs/component/animator.hpp>
#include <ecs/component/model.hpp>
#include <ecs/system/motion.hpp>
#include <ecs/system/world.hpp>

namespace detail {

void ConvertOzzToGlmMatrix(const ozz::math::Float4x4& ozz_matrix, glm::mat4& glm_matrix) {
    // Reinterpret cast the memory of ozz::math::Float4x4 to glm::mat4
    glm_matrix = reinterpret_cast<const glm::mat4&>(ozz_matrix);
}

void UpdateSkinnedPositions(
    const ozz::animation::Skeleton& skeleton,
    const ozz::animation::Animation& animation,
    float animation_time,
    const std::vector<glm::vec3>& tpose_positions,
    const std::vector<glm::uvec4>& bones, // 4 bone indices per vertex
    const std::vector<glm::vec4>& weights, // 4 weights per vertex
    std::vector<glm::vec3>& computed_positions)
{
    // Ensure the tpose_positions, bones, and weights have the same number of vertices
    assert(tpose_positions.size() == bones.size());
    assert(tpose_positions.size() == weights.size());

    // Step 2: Create and run the sampling job
    ozz::animation::SamplingJob sampling_job;
    sampling_job.animation = &animation;
    sampling_job.context->Resize(animation.num_tracks());

    std::vector<ozz::math::SoaTransform> local_transforms(skeleton.num_soa_joints());
    sampling_job.output = ozz::make_span(local_transforms);
    sampling_job.ratio = animation_time / animation.duration();
    sampling_job.Run();

    // Step 3: Convert local transforms to model space transforms
    std::vector<ozz::math::Float4x4> model_transforms(skeleton.num_joints());
    ozz::animation::LocalToModelJob ltm_job;
    ltm_job.skeleton = &skeleton;
    ltm_job.input = ozz::make_span(local_transforms);
    ltm_job.output = ozz::make_span(model_transforms);
    ltm_job.Run();

    // Step 4: Apply skinning to compute final positions
    computed_positions.resize(tpose_positions.size());

    for (size_t i = 0; i < tpose_positions.size(); ++i) {
        glm::vec4 skinned_position(0.0f); // Start with a zeroed position

        // Convert T-pose position to 4D vector for matrix multiplication
        glm::vec4 tpose_position(tpose_positions[i], 1.0f);

        // Accumulate contributions from each bone
        for (int j = 0; j < 4; ++j) {
            uint32_t bone_index = bones[i][j];
            float weight = weights[i][j];

            if (weight > 0.0f) {
                // Get the transformation matrix for this bone
                const ozz::math::Float4x4& bone_transform = model_transforms[bone_index];

                // Convert ozz::math::Float4x4 to glm::mat4
                glm::mat4 glm_bone_transform;
                ConvertOzzToGlmMatrix(bone_transform, glm_bone_transform);

                // Transform the T-pose position by the bone's transform matrix
                glm::vec4 transformed_position = glm_bone_transform * tpose_position;

                // Accumulate the weighted transformed position
                skinned_position += weight * transformed_position;
            }
        }

        // Store the resulting position in glm::vec3 format
        computed_positions[i] = glm::vec3(skinned_position);
    }
}

}

void motion_system::update()
{
    world_system::for_each([](entt::registry& _registry) {
        _registry.view<model_component, animator_component>().each([](model_component& _model, animator_component& _animator) {
            if (_model._mesh.has_value() && _animator._skeleton.has_value()) {
                
                

            }
        });
    });
}