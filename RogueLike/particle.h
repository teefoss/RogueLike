//
//  particle.h
//  RogueLike
//
//  Created by Thomas Foster on 4/7/23.
//

#ifndef particle_h
#define particle_h

#include "vector.h"
#include "inttypes.h"
#include <SDL.h>

typedef struct {
    vec2_t position;
    vec2_t velocity;
    SDL_Color color;
    u16 lifespan; // frame ticks
} particle_t;

typedef struct {
    particle_t * buffer;
    int buffer_size;
    int num_particles;
} particle_array_t;

void InitParticleArray(particle_array_t * array);
void InsertParticle(particle_array_t * array, particle_t particle);
void UpdateParticles(particle_array_t * array, float dt);
void RenderParticles(const particle_array_t * array, int draw_scale, vec2_t offset);

#endif /* particle_h */
