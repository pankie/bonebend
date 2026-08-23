//
// Created by freda on 2026-08-22.
//

#include "quaternion.h"

#include <math.h>

#include "SDL3/SDL_log.h"

quat_t quat_identity(void)
{
    return (quat_t){1.0f, 0.0f, 0.0f, 0.0f};
}

quat_t quat_from_mat4(const mat4_t *m)
{
    // Shamelessly stolen from quat_cast OpenGL Math library
    // https://github.com/g-truc/glm/blob/master/glm/gtc/quaternion.inl at row 81, quat_cast(mat<3, 3, T, Q> const& m)

    const float four_x_sq_m1 = m->m[0][0] - m->m[1][1] - m->m[2][2];
    const float four_y_sq_m1 = m->m[1][1] - m->m[0][0] - m->m[2][2];
    const float four_z_sq_m1 = m->m[2][2] - m->m[0][0] - m->m[1][1];
    const float four_w_sq_m1 = m->m[0][0] + m->m[1][1] + m->m[2][2];

    int biggest_index = 0;
    float four_biggest_sq_m1 = four_w_sq_m1;
    if (four_x_sq_m1 > four_biggest_sq_m1)
    {
        four_biggest_sq_m1 = four_x_sq_m1;
        biggest_index = 1;
    }
    if (four_y_sq_m1 > four_biggest_sq_m1)
    {
        four_biggest_sq_m1 = four_y_sq_m1;
        biggest_index = 2;
    }
    if (four_z_sq_m1 > four_biggest_sq_m1)
    {
        four_biggest_sq_m1 = four_z_sq_m1;
        biggest_index = 3;
    }

    const float biggest_val = sqrtf(four_biggest_sq_m1 + 1.0f) * 0.5f;
    const float mult = 0.25f / biggest_val;

    quat_t q;
    switch(biggest_index)
    {
        case 0:
            q.w = biggest_val;
            q.x = (m->m[2][1] - m->m[1][2]) * mult;
            q.y = (m->m[0][2] - m->m[2][0]) * mult;
            q.z = (m->m[1][0] - m->m[0][1]) * mult;
            break;
        case 1:
            q.w = (m->m[2][1] - m->m[1][2]) * mult;
            q.x = biggest_val;
            q.y = (m->m[1][0] + m->m[0][1]) * mult;
            q.z = (m->m[0][2] + m->m[2][0]) * mult;
            break;
        case 2:
            q.w = (m->m[0][2] - m->m[2][0]) * mult;
            q.x = (m->m[1][0] + m->m[0][1]) * mult;
            q.y = biggest_val;
            q.z = (m->m[2][1] + m->m[1][2]) * mult;
            break;
        case 3:
            q.w = (m->m[1][0] - m->m[0][1]) * mult;
            q.x = (m->m[0][2] + m->m[2][0]) * mult;
            q.y = (m->m[2][1] + m->m[1][2]) * mult;
            q.z = biggest_val;
            break;
        default:
            q = quat_identity();
            break;
    }

    return q;
}

quat_t quat_mul_quat(const quat_t p, const quat_t q)
{
    return (quat_t){
        p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z,
        p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y,
        p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z,
        p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x
    };
}

quat_t quat_mul_scalar(const quat_t p, const float s)
{
    return (quat_t){p.w * s, p.x * s, p.y * s, p.z * s};
}

float quat_inner_product(const quat_t p, const quat_t q)
{
    return p.w * q.w + p.x * q.x + p.y * q.y + p.z * q.z;
}

quat_t quat_add_quat(const quat_t p, const quat_t q)
{
    return (quat_t) { p.w + q.w, p.x + q.x, p.y + q.y, p.z + q.z };
}

quat_t quat_conjugate(const quat_t p)
{
    return (quat_t) {
        p.w, -p.x, -p.y, -p.z
    };
}

dual_quat_t dual_quat_from_mat4(const mat4_t *m)
{
    const quat_t real = quat_from_mat4(m);
    const quat_t t = (quat_t){0.0f, m->m[0][3], m->m[1][3], m->m[2][3]};
    const quat_t dual = quat_mul_scalar(quat_mul_quat(t, real), 0.5f); // q_d = 1/2 * t * q_r

    return (dual_quat_t){
        .real = real, .dual = dual
    };
}

dual_quat_t dual_quat_negate(const dual_quat_t *p)
{
    return (dual_quat_t){
        .real = quat_mul_scalar(p->real, -1), .dual = quat_mul_scalar(p->dual, -1)
    };
}

dual_quat_t dual_quat_add(const dual_quat_t *p, const dual_quat_t *q)
{
    return (dual_quat_t) {
        .real = quat_add_quat(p->real, q->real), .dual = quat_add_quat(p->dual, q->dual)
    };
}

dual_quat_t dual_quat_mul_scalar(const dual_quat_t *p, const float s)
{
    return (dual_quat_t) {
        .real = quat_mul_scalar(p->real, s), .dual = quat_mul_scalar(p->dual, s)
    };
}

mat4_t dual_quat_to_mat4(const dual_quat_t *dual_quat)
{
    // Shamelessly stolen from mat3_cast (mat4_cast, more specifically) OpenGL Math library
    // https://github.com/g-truc/glm/blob/master/glm/gtc/quaternion.inl at row 47, mat3_cast(qua<T, Q> const& q)
    const quat_t* q_r = &dual_quat->real;
    const quat_t* q_d = &dual_quat->dual;

    // translation
    const quat_t t = quat_mul_scalar(quat_mul_quat(*q_d, quat_conjugate(*q_r) ), 2.0f); // t = 2*q_d * q_r^* (q_r^* conjugate of q_r)
    const mat4_t translation = mat4_make_translation(t.x, t.y, t.z);

    if (fabsf(t.w) > 0.001f) {
        SDL_Log("dual_quat_to_mat4: unexpected nonzero t.w = %f, dual quaternion may be invalid", t.w);
    }

    // rotation
    mat4_t rotation = mat4_identity();
    const float qxx = q_r->x * q_r->x;
    const float qyy = q_r->y * q_r->y;
    const float qzz = q_r->z * q_r->z;
    const float qxz = q_r->x * q_r->z;
    const float qxy = q_r->x * q_r->y;
    const float qyz = q_r->y * q_r->z;
    const float qwx = q_r->w * q_r->x;
    const float qwy = q_r->w * q_r->y;
    const float qwz = q_r->w * q_r->z;

    rotation.m[0][0] = 1.0f - 2.0f * (qyy + qzz);
    rotation.m[0][1] = 2.0f * (qxy - qwz);
    rotation.m[0][2] = 2.0f * (qxz + qwy);

    rotation.m[1][0] = 2.0f * (qxy + qwz);
    rotation.m[1][1] = 1.0f - 2.0f * (qxx + qzz);
    rotation.m[1][2] = 2.0f * (qyz - qwx);

    rotation.m[2][0] = 2.0f * (qxz - qwy);
    rotation.m[2][1] = 2.0f * (qyz + qwx);
    rotation.m[2][2] = 1.0f - 2.0f * (qxx + qyy);

    return mat4_mul_mat4(&translation, &rotation);
}