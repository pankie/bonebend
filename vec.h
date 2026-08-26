//
// Created by freda on 2026-08-20.
//

#ifndef BONEBEND_VEC_H
#define BONEBEND_VEC_H

typedef struct {
    float x, y, z;
} vec3_t;

typedef struct {
    float x, y, z, w;
} vec4_t;

vec4_t vec4_from_vec3(vec3_t v);   // (x, y, z) -> (x, y, z, 1)
vec3_t vec3_from_vec4(vec4_t v);   // drop the homogeneous component, i.e. (x, y, z, 1) -> (x, y, z)
vec4_t vec4_add(vec4_t a, vec4_t b);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_cross(vec3_t a, vec3_t b);
vec3_t normalize(vec3_t v);
vec4_t vec4_mul_scalar(vec4_t v, float s);
vec3_t vec3_mul_scalar(vec3_t v, float s);

#endif //BONEBEND_VEC_H
