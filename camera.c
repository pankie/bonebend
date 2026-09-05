//
// Created by freda on 2026-09-05.
//

#include "camera.h"

#include <math.h>

camera_t construct_camera(float fov, const float aspect, const float near_plane, float far_plane)
{
    const float min_tolerance = 0.01f;
    fov = fmaxf(fov, min_tolerance);
    far_plane = fmaxf(far_plane, near_plane + min_tolerance);
    const float tan_half_theta_inverse = 1.0f / tanf(fov * 0.5f);

    mat4_t projection_matrix = mat4_identity();
    projection_matrix.m[0][0] = tan_half_theta_inverse * aspect; // we assume aspect is defined as height / width
    projection_matrix.m[1][1] = tan_half_theta_inverse;
    projection_matrix.m[2][2] = -(far_plane + near_plane) / (far_plane - near_plane);
    projection_matrix.m[2][3] = -2.0f * far_plane * near_plane / (far_plane - near_plane);
    projection_matrix.m[3][2] = -1.0f;
    projection_matrix.m[3][3] = 0.0f;

    return (camera_t) {
        .projection = projection_matrix,
        .direction = (vec3_t) {0.0f, 0.0f, -1.0f},
        .up = (vec3_t) {0.0f, 1.0f, 0.0f},
    };
}

void update_view_camera(camera_t *camera)
{
    const vec3_t world_up = { 0.0f, 1.0f, 0.0f };

    const vec3_t forward = vec3_normalize(camera->direction);
    const vec3_t right = vec3_normalize(vec3_cross(forward, world_up));
    const vec3_t up = vec3_normalize(vec3_cross(right, forward));

    mat4_t view_matrix = mat4_identity();
    view_matrix.m[0][0] = right.x;
    view_matrix.m[0][1] = right.y;
    view_matrix.m[0][2] = right.z;
    view_matrix.m[1][0] = up.x;
    view_matrix.m[1][1] = up.y;
    view_matrix.m[1][2] = up.z;
    view_matrix.m[2][0] = -forward.x;
    view_matrix.m[2][1] = -forward.y;
    view_matrix.m[2][2] = -forward.z;

    view_matrix.m[0][3] = -vec3_dot_product(right, camera->position);
    view_matrix.m[1][3] = -vec3_dot_product(up, camera->position);
    view_matrix.m[2][3] = vec3_dot_product(forward, camera->position);

    camera->view = view_matrix;
    camera->up = up;
}
