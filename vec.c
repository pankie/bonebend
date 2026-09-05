//
// Created by freda on 2026-08-20.
//

#include "vec.h"

#include <math.h>
#include <stdint.h>

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

vec3_t vec3_add(vec3_t a, vec3_t b)
{
    const vec3_t result = { a.x + b.x, a.y + b.y, a.z + b.z };
    return result;
}

vec3_t vec3_sub(vec3_t a, vec3_t b)
{
    const vec3_t result = {a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

vec3_t vec3_cross(const vec3_t a, const vec3_t b)
{
    return (vec3_t) {
        a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x
    };
}

float vec3_dot_product(const vec3_t a, const vec3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3_t vec3_normalize(const vec3_t v)
{
    const float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return (vec3_t){ v.x / length, v.y / length, v.z / length };
}


vec4_t vec4_mul_scalar(const vec4_t v, const float s) {
    const vec4_t result = { v.x * s, v.y * s, v.z * s, v.w * s };
    return result;
}

vec3_t vec3_mul_scalar(const vec3_t v, const float s)
{
    const vec3_t result = { v.x * s, v.y * s, v.z * s };
    return result;
}

vec2_t vec2_sub(vec2_t a, vec2_t b)
{
    const vec2_t result = { a.x - b.x, a.y - b.y };
    return result;
}

vec3_t barycentric_weights(const vec2_t a, const vec2_t b, const vec2_t c, const vec2_t p)
{
    const vec2_t ac = vec2_sub(c, a);
    const vec2_t ab = vec2_sub(b, a);
    const vec2_t pc = vec2_sub(c, p);
    const vec2_t pb = vec2_sub(b, p);
    const vec2_t ap = vec2_sub(p, a);

    const float area_parallelogram_abc = ac.x * ab.y - ac.y * ab.x; // || AC x AB ||
    const float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc;
    const float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc;
    const float gamma = 1 - alpha - beta;

    return (vec3_t) {
        alpha, beta, gamma
    };
}
