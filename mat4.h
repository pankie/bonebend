//
// Created by freda on 2026-08-21.
//

#ifndef BONEBEND_MAT4_H
#define BONEBEND_MAT4_H

#include "vec.h"

typedef struct {
    float m[4][4];
} mat4_t;

mat4_t mat4_identity(void);
mat4_t mat4_make_translation(float tx, float ty, float tz);   // T(tx, ty, tz)
mat4_t mat4_make_rotation_z(float angle_rad);                 // R_z(theta)
mat4_t mat4_make_rotation_y(float angle_rad);                 // R_y(theta)
mat4_t mat4_mul_mat4(const mat4_t* a, const mat4_t* b);
vec4_t mat4_mul_vec4(const mat4_t* m, vec4_t v);

// Inverse for RIGID transforms only (rotation + translation, no scale) -
// uses the block-matrix shortcut derived in the notes
//   M^-1 = [ R^T   -R^T t ]
//          [ 0^T      1  ]
mat4_t mat4_rigid_inverse(const mat4_t* m);

#endif //BONEBEND_MAT4_H
