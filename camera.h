//
// Created by freda on 2026-09-05.
//

#ifndef BONEBEND_CAMERA_H
#define BONEBEND_CAMERA_H
#include "mat4.h"

typedef struct
{
    mat4_t projection;
    mat4_t view;
    vec3_t position;
    vec3_t direction;
    vec3_t up;
} camera_t;

camera_t construct_camera(float fov, float aspect, float near_plane, float far_plane);
void update_view_camera(camera_t *camera);

#endif //BONEBEND_CAMERA_H
