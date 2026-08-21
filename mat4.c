//
// Created by freda on 2026-08-21.
//

#include "mat4.h"
#include <math.h>

mat4_t mat4_identity(void) {
    mat4_t m = {0};
    m.m[0][0] = 1.0f;
    m.m[1][1] = 1.0f;
    m.m[2][2] = 1.0f;
    m.m[3][3] = 1.0f;
    return m;
}

mat4_t mat4_make_translation(const float tx, const float ty, const float tz) {
    mat4_t m = mat4_identity();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    return m;
}

mat4_t mat4_make_rotation_z(const float angle_rad) {
    const float c = cosf(angle_rad);
    const float s = sinf(angle_rad);
    mat4_t m = mat4_identity();
    m.m[0][0] = c;  m.m[0][1] = -s;
    m.m[1][0] = s;  m.m[1][1] = c;
    return m;
}

mat4_t mat4_mul_mat4(const mat4_t* a, const mat4_t* b) {
    mat4_t result = {0};
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += a->m[row][k] * b->m[k][col];
            }
            result.m[row][col] = sum;
        }
    }
    return result;
}

vec4_t mat4_mul_vec4(const mat4_t* m, const vec4_t* v) {
    vec4_t result;
    result.x = m->m[0][0]*v->x + m->m[0][1]*v->y + m->m[0][2]*v->z + m->m[0][3]*v->w;
    result.y = m->m[1][0]*v->x + m->m[1][1]*v->y + m->m[1][2]*v->z + m->m[1][3]*v->w;
    result.z = m->m[2][0]*v->x + m->m[2][1]*v->y + m->m[2][2]*v->z + m->m[2][3]*v->w;
    result.w = m->m[3][0]*v->x + m->m[3][1]*v->y + m->m[3][2]*v->z + m->m[3][3]*v->w;
    return result;
}

mat4_t mat4_rigid_inverse(const mat4_t* m) {
    mat4_t result = mat4_identity();

    // R^T: transpose the 3x3 rotation block
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            result.m[row][col] = m->m[col][row];
        }
    }

    // -R^T * t
    const float tx = m->m[0][3];
    const float ty = m->m[1][3];
    const float tz = m->m[2][3];

    result.m[0][3] = -(result.m[0][0]*tx + result.m[0][1]*ty + result.m[0][2]*tz);
    result.m[1][3] = -(result.m[1][0]*tx + result.m[1][1]*ty + result.m[1][2]*tz);
    result.m[2][3] = -(result.m[2][0]*tx + result.m[2][1]*ty + result.m[2][2]*tz);

    return result;
}