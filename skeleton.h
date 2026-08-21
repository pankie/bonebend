//
// Created by freda on 2026-08-21.
//

#ifndef BONEBEND_SKELETON_H
#define BONEBEND_SKELETON_H
#include <stdint.h>

#include "mat4.h"

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
} skeleton_t;

extern skeleton_t skeleton;

void init_skeleton(int32_t bone_count, float segment_length);
void skeleton_update_pose(mat4_t* out_skin_matrices, float time);
vec3_t skin_vertex_single_bone(vec3_t bind_v, const mat4_t* skin_matrix);
vec3_t get_bone_position(const mat4_t* global_bind_transform, const mat4_t* skin_matrix, const mat4_t* world_matrix);
vec3_t get_bone_point_world(vec3_t local_offset, const mat4_t* global_bind_transform, const mat4_t* skin_matrix, const mat4_t* world_matrix);

#endif //BONEBEND_SKELETON_H
