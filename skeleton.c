//
// Created by freda on 2026-08-21.
//

#include "skeleton.h"

skeleton_t skeleton = {
    .bones_count = 0
};

void init_skeleton(const int32_t bone_count, const float segment_length)
{
    int32_t parent_index = -1;
    mat4_t local_transform = mat4_identity();
    mat4_t global_bind_transform = mat4_identity();
    mat4_t inverse_bind_pose = mat4_identity();

    const mat4_t T = mat4_make_translation(0.0f, 0.0f, 0.0f);
    const mat4_t R = mat4_make_rotation_z(0.0f);

    // root bone has same global bind transform as local transform
    local_transform = mat4_mul_mat4(&T, &R);
    global_bind_transform = local_transform; // mat4_mul_mat4(&local_transform, I);
    inverse_bind_pose = mat4_rigid_inverse(&global_bind_transform);

    const bone_t bone = {
        .parent_index = parent_index,
        .local_transform = local_transform,
        .global_bind_transform = global_bind_transform,
        .inverse_bind_pose = inverse_bind_pose,
    };

    skeleton.bones_count = 1;
    skeleton.bones[0] = bone;
}

void skeleton_update_pose(mat4_t* out_skin_matrices, const float time)
{
    const mat4_t L = mat4_make_rotation_z(time * 0.025f);
    const mat4_t S = mat4_mul_mat4(&L, &skeleton.bones[0].inverse_bind_pose);
    out_skin_matrices[0] = S;
}

vec3_t skin_vertex_single_bone(const vec3_t bind_v, const mat4_t *skin_matrix)
{
    const vec4_t converted = vec4_from_vec3(bind_v);
    const vec4_t result = mat4_mul_vec4(skin_matrix, converted);
    return vec3_from_vec4(result);
}

vec3_t get_bone_position(const mat4_t *global_bind_transform, const mat4_t *skin_matrix, const mat4_t *world_matrix)
{
    return get_bone_point_world((vec3_t){0, 0, 0}, global_bind_transform, skin_matrix, world_matrix);
}

vec3_t get_bone_point_world(const vec3_t local_offset, const mat4_t *global_bind_transform, const mat4_t *skin_matrix,
    const mat4_t *world_matrix)
{
    const vec4_t v_bind = mat4_mul_vec4(global_bind_transform, vec4_from_vec3(local_offset));
    const vec4_t v_skinned = mat4_mul_vec4(skin_matrix, v_bind);
    const vec4_t v_world = mat4_mul_vec4(world_matrix, v_skinned);
    return vec3_from_vec4(v_world);
}
