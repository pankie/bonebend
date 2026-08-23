//
// Created by freda on 2026-08-21.
//

#ifndef BONEBEND_SKELETON_H
#define BONEBEND_SKELETON_H
#include <stdint.h>

#include "mat4.h"
#include "quaternion.h"

#define MAX_BONES 16

/*
 * Similar to the notes, we have
 * local_transform (or local bind)  -> L_i      : bone i's rest-pose transform relative to its parent
 * global_bind_transform            -> W_i      : bone i's rest-pose transform in model space
 * inverse_bind_pose                -> W_i^{-1} : cached inverse of global_bind_transform
 */
typedef struct
{
    int32_t parent_index; // -1 for the root bone
    mat4_t local_transform;
    mat4_t global_bind_transform;
    mat4_t inverse_bind_pose;
} bone_t;

typedef struct
{
    bone_t bones[MAX_BONES];
    uint32_t bones_count;
    mat4_t skin_matrices[MAX_BONES]; // skin matrices S_i for the current frame
    dual_quat_t dual_quaternions[MAX_BONES]; // used exclusively for dual quaternion skinning
} skeleton_t;

extern skeleton_t skeleton;

void init_skeleton(int32_t bone_count, float segment_length);
void skeleton_update_pose(mat4_t* out_skin_matrices, float time); // linear blend skinning
void skeleton_update_pose_dual_quat(dual_quat_t* out_dual_quaternions, float time); // dual quaternion skinning

// Linear Blend Skinning
vec3_t skin_vertex_lbs(
    vec3_t v_bind, int32_t bone_a, int32_t bone_b, float weight_a, const mat4_t* skin_matrices
);

// Dual Quaternion Blending
dual_quat_t dual_quat_blend(int32_t bone_a, int32_t bone_b, float weight_a, const dual_quat_t* dual_quaternions);
vec3_t skin_vertex_dqs(vec3_t v_bind, int32_t bone_a, int32_t bone_b, float weight_a, const dual_quat_t* dual_quaternions);

vec3_t skin_vertex_single_bone(vec3_t v_bind, const mat4_t* skin_matrix);
vec3_t get_bone_position(const mat4_t* global_bind_transform, const mat4_t* skin_matrix, const mat4_t* world_matrix);
vec3_t get_bone_point_world(vec3_t local_offset, const mat4_t* global_bind_transform, const mat4_t* skin_matrix, const mat4_t* world_matrix);

#endif //BONEBEND_SKELETON_H
