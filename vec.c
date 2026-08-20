//
// Created by freda on 2026-08-20.
//

#include "vec.h"

vec4_t vec4_from_vec3(const vec3_t v) {
    const vec4_t result = { v.x, v.y, v.z, 1.0f };
    return result;
}

vec3_t vec3_from_vec4(const vec4_t v) {
    const vec3_t result = { v.x, v.y, v.z };
    return result;
}

vec4_t vec4_add(const vec4_t a, const vec4_t b) {
    const vec4_t result = { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
    return result;
}

vec4_t vec4_mul_scalar(const vec4_t v, const float s) {
    const vec4_t result = { v.x * s, v.y * s, v.z * s, v.w * s };
    return result;
}
