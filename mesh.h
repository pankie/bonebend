//
// Created by freda on 2026-08-21.
//

#ifndef BONEBEND_MESH_H
#define BONEBEND_MESH_H

#define MAX_BOX_VERTICES 256
#define MAX_BOX_FACES 512

#include "triangle.h"
#include "vec.h"

extern uint32_t triangle_count;

typedef struct
{
    vec3_t vertices[MAX_BOX_VERTICES];
    uint32_t vertices_count;

    face_t faces[MAX_BOX_VERTICES];
    uint32_t faces_count;

    vec3_t rotation;
    vec3_t translation;
} mesh_t;

extern mesh_t mesh;

void init_mesh(int32_t ring_count, float length, float half_width, float half_depth);

#endif //BONEBEND_MESH_H
