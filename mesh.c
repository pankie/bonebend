//
// Created by freda on 2026-08-21.
//

#include "mesh.h"

#include <math.h>

#include "SDL3/SDL_log.h"

mesh_t mesh = {
    .rotation = {0, 0, 0},
    .translation = {0, 0, 0},
};

void init_mesh(const int32_t ring_count, const float length, const float half_width, const float half_depth)
{
    mesh.vertices_count = 0;
    mesh.faces_count = 0;

    // 4 corners per ring, stacked along y from 0 to length
    for (int32_t r = 0; r < ring_count; r++)
    {
        const float y = length * (float) r / (float)(ring_count - 1);
        const vec3_t corners[4] = {
            { -half_width, y, -half_depth },
            {  half_width, y, -half_depth },
            {  half_width, y,  half_depth },
            { -half_width, y,  half_depth },
        };

        for (size_t c = 0; c < 4; c++)
        {
            mesh.vertices[mesh.vertices_count++] = corners[c];
        }
    }

    // 2 triangles per quad, between each pair of adjacent rings
    for (int32_t r = 0; r < ring_count - 1; r++)
    {
        // ring bases
        const int32_t base = r * 4;
        const int32_t next_base = base + 4;

        for (int32_t c = 0; c < 4; c++)
        {
            int32_t c_next = (c + 1) % 4;

            // quad corners
            const int32_t v0 = base + c;
            const int32_t v1 = base + c_next;
            const int32_t v2 = next_base + c_next;
            const int32_t v3 = next_base + c;

            // two triangles per quad, with face_t indices being 1 index based
            mesh.faces[mesh.faces_count].a = v0 + 1;
            mesh.faces[mesh.faces_count].b = v1 + 1;
            mesh.faces[mesh.faces_count].c = v2 + 1;
            mesh.faces_count++;

            mesh.faces[mesh.faces_count].a = v0 + 1;
            mesh.faces[mesh.faces_count].b = v2 + 1;
            mesh.faces[mesh.faces_count].c = v3 + 1;
            mesh.faces_count++;
        }
    }

    // add missing bottom cap
    mesh.faces[mesh.faces_count].a = 0 + 1;
    mesh.faces[mesh.faces_count].b = 1 + 1;
    mesh.faces[mesh.faces_count].c = 2 + 1;
    mesh.faces_count++;

    mesh.faces[mesh.faces_count].a = 0 + 1;
    mesh.faces[mesh.faces_count].b = 2 + 1;
    mesh.faces[mesh.faces_count].c = 3 + 1;
    mesh.faces_count++;

    // and add top cap, facing outwards
    const int32_t top_base = (ring_count - 1) * 4;
    mesh.faces[mesh.faces_count].a = top_base + 1 + 1;
    mesh.faces[mesh.faces_count].b = top_base + 0 + 1;
    mesh.faces[mesh.faces_count].c = top_base + 3 + 1;
    mesh.faces_count++;

    mesh.faces[mesh.faces_count].a = top_base + 2 + 1;
    mesh.faces[mesh.faces_count].b = top_base + 1 + 1;
    mesh.faces[mesh.faces_count].c = top_base + 3 + 1;
    mesh.faces_count++;

}

void assign_weights(const float y, const float segment_length, const int32_t bone_count, int32_t *bone_a, int32_t *bone_b,
    float *weight_a)
{
    const float segment_fraction = y / segment_length;
    int32_t a = (int32_t) floorf(segment_fraction);

    // clamp index a so that we don't run out of bounds of bone index
    if (a < 0) a = 0;
    if (a > bone_count - 2) a = bone_count - 2;

    *bone_a = a;
    *bone_b = a + 1;

    float weight = 1.0f - (segment_fraction - (float) a);
    if (weight < 0.0f) weight = 0.0f;
    if (weight > 1.0f) weight = 1.0f;

    *weight_a = weight;
}

void assign_mesh_weights(const int32_t bone_count, const float segment_length)
{
    SDL_Log("Assigning mesh weights for %d verices", mesh.vertices_count);
    for (uint32_t i = 0; i < mesh.vertices_count; i++)
    {
        assign_weights(mesh.vertices[i].y, segment_length, bone_count, &mesh.bone_a[i], &mesh.bone_b[i], &mesh.weight_a[i]);
    }
}
