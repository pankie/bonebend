//
// Created by freda on 2026-08-21.
//

#include "skeleton.h"

#include <math.h>
#include <SDL3/SDL_log.h>

skeleton_t skeleton = {
    .bones_count = 0
};

void init_skeleton(const int32_t bone_count, const float segment_length)
{
    // root bone being identity rigid transform
    skeleton.bones[0] = (bone_t) {
        .parent_index = -1,
        .local_transform = mat4_identity(),
        .global_bind_transform = mat4_identity(),
        .inverse_bind_pose = mat4_identity(),
    };

    skeleton.bones_count++;

    // attach the chain of bones with each 'i' being parent to 'i - 1'
    for (int32_t i = 1; i < bone_count; i++)
    {
        const bone_t* parent_bone = &skeleton.bones[i - 1];
        const mat4_t local_transform = mat4_make_translation(0, segment_length, 0);
        const mat4_t global_bind_transform = mat4_mul_mat4(&parent_bone->global_bind_transform, &local_transform);
        const mat4_t inverse_bind_pose = mat4_rigid_inverse(&global_bind_transform);

        skeleton.bones[i] = (bone_t) {
            .parent_index = i - 1,
            .local_transform = local_transform,
            .global_bind_transform = global_bind_transform,
            .inverse_bind_pose = inverse_bind_pose,
        };

        skeleton.bones_count++;
    }

    SDL_Log("Skeleton initialized with %d bones with segment length %.2f", skeleton.bones_count, segment_length);
}

void skeleton_update_pose(mat4_t* out_skin_matrices, const float time)
{
    // W_i^{new}, this frame, per bone
    mat4_t world_pose[MAX_BONES];
    for (size_t i = 0; i < skeleton.bones_count; i++)
    {
        const bone_t* bone = &skeleton.bones[i];
        const mat4_t R = mat4_make_rotation_z(time * 0.25f);

        const mat4_t L_new = mat4_mul_mat4(&bone->local_transform, &R); // L_i^{new}

        if (bone->parent_index != -1)
        {
            // W_i^{new} = W_{p_i}^{new} * L_i^{new}
            world_pose[i] = mat4_mul_mat4(&world_pose[bone->parent_index], &L_new);
        }
        else
        {
            // W_0^{new} = L_0^{new}
            world_pose[i] = L_new;
        }

        // S_i = W_i^{new} * W_i^{-1}
        out_skin_matrices[i] = mat4_mul_mat4(&world_pose[i], &bone->inverse_bind_pose);
    }
}

vec3_t skin_vertex_lbs(const vec3_t v_bind, const int32_t bone_a, const int32_t bone_b, const float weight_a, const mat4_t *skin_matrices)
{
    const float weight_b = 1 - weight_a;

    // v_skinned =  w_a * S_a * v_bind + w_b * S_b * v_bind
    const vec3_t v_skinned = vec3_add(
        vec3_mul_scalar(skin_vertex_single_bone(v_bind, &skin_matrices[bone_a]), weight_a),
        vec3_mul_scalar(skin_vertex_single_bone(v_bind, &skin_matrices[bone_b]), weight_b)
    );

    return v_skinned;
}

dual_quat_t dual_quat_blend(const int32_t bone_a, const int32_t bone_b, const float weight_a, const dual_quat_t *dual_quaternions)
{
    const float weight_b = 1.0f - weight_a;

    // 1. Sign consistency
    const size_t bone_ref = 0; // we always pick first bone as reference
    const dual_quat_t* dual_quat_ref = &dual_quaternions[bone_ref];

    dual_quat_t dual_quat_a = dual_quaternions[bone_a];
    if (bone_ref != bone_a && quat_inner_product(dual_quat_a.real, dual_quat_ref->real) < 0)
    {
        dual_quat_a = dual_quat_negate(&dual_quat_a);
    }

    dual_quat_t dual_quat_b = dual_quaternions[bone_b];
    if (bone_ref != bone_b && quat_inner_product(dual_quat_b.real, dual_quat_ref->real) < 0)
    {
        dual_quat_b = dual_quat_negate(&dual_quat_b);
    }

    // 2. Weighted sum
    const dual_quat_t dual_quat_a_w = dual_quat_mul_scalar(&dual_quat_a, weight_a);
    const dual_quat_t dual_quat_b_w = dual_quat_mul_scalar(&dual_quat_b, weight_b);
    const dual_quat_t b_hat = dual_quat_add(&dual_quat_a_w, &dual_quat_b_w);

    // 3. Normalize (using only real part)
    const float b_hat_norm = sqrtf( quat_inner_product(b_hat.real, b_hat.real) );
    const float b_hat_norm_inverse = 1.f / b_hat_norm;

    return (dual_quat_t) {
        .real = quat_mul_scalar(b_hat.real, b_hat_norm_inverse), .dual = quat_mul_scalar(b_hat.dual, b_hat_norm_inverse)
    };
}

vec3_t skin_vertex_single_bone(const vec3_t v_bind, const mat4_t *skin_matrix)
{
    const vec4_t v_skinned = mat4_mul_vec4(skin_matrix, vec4_from_vec3(v_bind));
    return vec3_from_vec4(v_skinned);
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
