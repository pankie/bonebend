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
    // Shamelessly stolen from quat_cast OpenGL Math library, without the voting system:
    // https://github.com/g-truc/glm/blob/master/glm/gtc/quaternion.inl at row 81, quat_cast(mat<3, 3, T, Q> const& m)

    // note the only difference is that we consider only z-rotation, the trace > 0 branch will always be the one taken
    const float trace = m->m[0][0] + m->m[1][1] + m->m[2][2]; // fourWSquaredMinus1
    quat_t q;
    if (trace > 0.0f)
    {
        const float s = sqrtf(trace + 1.0f)*2.0f;
        q.w = 0.25f * s;
        q.x = (m->m[2][1] - m->m[1][2]) / s;
        q.y = (m->m[0][2] - m->m[2][0]) / s;
        q.z = (m->m[1][0] - m->m[0][1]) / s;
    }
    else
    {
        SDL_Log("quat_from_mat4: unexpected trace <= 0, quaternion may be invalid");
        q = quat_identity();
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

dual_quat_t dual_quat_from_mat4(const mat4_t *m)
{
    const quat_t real = quat_from_mat4(m);
    const quat_t t = (quat_t){0.0f, m->m[0][3], m->m[1][3], m->m[2][3]};
    const quat_t dual = quat_mul_scalar(quat_mul_quat(t, real), 0.5f); // q_d = 1/2 * t * q_r

    return (dual_quat_t){
        .real = real, .dual = dual
    };
}