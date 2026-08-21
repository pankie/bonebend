//
// Created by freda on 2026-08-21.
//

#ifndef BONEBEND_MESH_H
#define BONEBEND_MESH_H

#define N_CUBE_VERTICES 8
#define N_CUBE_MESH_FACES (6 * 2)
#include "triangle.h"
#include "vec.h"

extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_MESH_FACES];
extern uint32_t triangle_count;

typedef struct
{
    vec3_t* vertices;
    face_t* faces;
    vec3_t rotation;
    vec3_t scale;
    vec3_t translation;
} mesh_t;

extern mesh_t mesh;

void init_mesh(void);

#endif //BONEBEND_MESH_H
