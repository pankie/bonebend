//
// Created by freda on 2026-08-21.
//

#ifndef BONEBEND_MESH_H
#define BONEBEND_MESH_H

#define MAX_BOX_VERTICES 256
#define MAX_BOX_FACES 512

#include "skeleton.h"
#include "triangle.h"
#include "vec.h"

typedef struct
{
    vec3_t vertices[MAX_BOX_VERTICES];
    uint32_t vertices_count;

    face_t faces[MAX_BOX_FACES];
    uint32_t faces_count;

    int32_t bone_a [MAX_BOX_VERTICES];
    int32_t bone_b [MAX_BOX_VERTICES];
    float weight_a[MAX_BOX_VERTICES];

    vec3_t rotation;
    vec3_t translation;
} mesh_t;

extern mesh_t mesh;

void init_mesh(int32_t ring_count, float length, float half_width, float half_depth, chain_axis_t chain_axis);
void assign_weights(float y, float segment_length, int32_t bone_count, int32_t* bone_a, int32_t* bone_b, float* weight_a);
void assign_mesh_weights(int32_t bone_count, float segment_length, chain_axis_t chain_axis);

typedef void (*build_corners_fn)(float w, float half_width, float half_depth, vec3_t* corners);
void build_y_corners(float y, float half_width, float half_depth, vec3_t* corners);
void build_z_corners(float z, float half_width, float half_depth, vec3_t* corners);

#endif //BONEBEND_MESH_H
