//
// Created by freda on 2026-08-21.
//

#ifndef BONEBEND_TRIANGLE_H
#define BONEBEND_TRIANGLE_H
#include <stdint.h>
#include "vec.h"

typedef struct
{
    int32_t a, b, c;
} face_t;

typedef struct
{
    vec4_t points[3];
} triangle_t;

#endif //BONEBEND_TRIANGLE_H
