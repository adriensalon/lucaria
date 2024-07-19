#pragma once

constexpr short bulletgroupID_kinematic_rigidbody = 1 << 0; // interacts with ?collider_ground, ?collider_wall, ?collider_layerX
constexpr short bulletgroupID_dynamic_rigidbody = 1 << 1; // interacts with ?dynamic_rigidbody, collider_ground, collider_wall
constexpr short bulletgroupID_collider_ground = 1 << 2; // interacts with kinematic_rigidbody, dynamic_rigidbody
constexpr short bulletgroupID_collider_wall = 1 << 3; // interacts with kinematic_rigidbody, dynamic_rigidbody
constexpr short bulletgroupID_collider_layer_0 = 1 << 4; // interacts with kinematic_rigidbody
constexpr short bulletgroupID_collider_layer_1 = 1 << 5; // interacts with kinematic_rigidbody
constexpr short bulletgroupID_collider_layer_2 = 1 << 6; // interacts with kinematic_rigidbody
constexpr short bulletgroupID_collider_layer_3 = 1 << 7; // interacts with kinematic_rigidbody
constexpr short bulletgroupID_collider_layer_4 = 1 << 8; // interacts with kinematic_rigidbody
constexpr short bulletgroupID_collider_layer_5 = 1 << 9; // interacts with kinematic_rigidbody
constexpr short bulletgroupID_collider_layer_6 = 1 << 10; // interacts with kinematic_rigidbody
constexpr short bulletgroupID_collider_layer_7 = 1 << 11; // interacts with kinematic_rigidbody

enum struct kinematic_layer : short {
    layer_0 = bulletgroupID_collider_layer_0,
    layer_1 = bulletgroupID_collider_layer_1,
    layer_2 = bulletgroupID_collider_layer_2,
    layer_3 = bulletgroupID_collider_layer_3,
    layer_4 = bulletgroupID_collider_layer_4,
    layer_5 = bulletgroupID_collider_layer_5,
    layer_6 = bulletgroupID_collider_layer_6,
    layer_7 = bulletgroupID_collider_layer_7,
};

inline bool contains_layer(const short containing, const short contained) 
{
    return (containing & contained) == contained; 
}

inline short remove_layer(const short containing, const short to_remove) 
{
    return containing & ~to_remove;
}