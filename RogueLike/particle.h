//
//  particle.h
//  RogueLike
//
//  Created by Thomas Foster on 4/7/23.
//

#ifndef particle_h
#define particle_h

#include "vector.h"
#include "shorttypes.h"
#include <SDL.h>

typedef struct {
    vec2_t position;
    vec2_t velocity;
    SDL_Color color;
    u16 lifespan; // frame ticks
} Particle;

typedef struct {
    Particle * buffer;
    int buffer_size;
    int num_particles;
} ParticleArray;

void InitParticleArray(ParticleArray * array);
void InsertParticle(ParticleArray * array, Particle particle);
void UpdateParticles(ParticleArray * array, float dt);
void RenderParticles(const ParticleArray * array, int draw_scale, vec2_t offset);

#endif /* particle_h */
