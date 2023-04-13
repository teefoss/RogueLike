//
//  particle.c
//  RogueLike
//
//  Created by Thomas Foster on 4/7/23.
//

#include "particle.h"
#include "video.h"

void InitParticleArray(particle_array_t * array)
{
    if ( array->buffer ) {
        free(array->buffer);
        array->buffer = NULL;
    }

    array->buffer_size = 1024 * sizeof(*array->buffer);
    array->buffer = malloc(array->buffer_size);
    array->num_particles = 0;
}


void InsertParticle(particle_array_t * array, particle_t particle)
{
    int new_size = (array->num_particles + 1) * sizeof(particle);

    if ( new_size > array->buffer_size ) {
        array->buffer_size *= 2;
        array->buffer = realloc(array->buffer, array->buffer_size);
    }

    array->buffer[array->num_particles++] = particle;
}


void UpdateParticles(particle_array_t * array, float dt)
{
    for ( int i = array->num_particles - 1; i >= 0; i-- ) {
        particle_t * p = array->buffer + i;

        // Remove?
        if ( --p->lifespan == 0 ) {
            array->buffer[i] = array->buffer[--array->num_particles];
        }

        vec2_t velocity = Vec2Scale(p->velocity, dt);
        p->position = Vec2Add(p->position, velocity);
    }
}


void RenderParticles(const particle_array_t * array, int draw_scale, vec2_t offset)
{
    SDL_Rect r = { .w = draw_scale, .h = draw_scale };

    for ( int i = 0; i < array->num_particles; i++ ) {
        const particle_t * p = array->buffer + i;

        r.x = p->position.x * draw_scale - draw_scale / 2 - offset.x;
        r.y = p->position.y * draw_scale - draw_scale / 2 - offset.y;

        V_SetColor(p->color);
        V_FillRect(&r);
    }
}
