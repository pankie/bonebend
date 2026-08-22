//
// Created by freda on 2026-08-22.
//

#ifndef BONEBEND_QUATERNION_H
#define BONEBEND_QUATERNION_H
#include "mat4.h"

typedef struct
{
    float w, x, y, z; // similar to GLM, the first component is real part while rest are the imaginary
} quat_t;

typedef struct
{
    quat_t real; // q_r - rotation
    quat_t dual; // q_d - translation
} dual_quat_t;

quat_t quat_identity(void);
quat_t quat_from_mat4(const mat4_t* m);
quat_t quat_mul_quat(quat_t p, quat_t q);
quat_t quat_mul_scalar(quat_t p, float s);

dual_quat_t dual_quat_from_mat4(const mat4_t* m);

#endif //BONEBEND_QUATERNION_H
