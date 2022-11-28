//
//  vector.c
//  Game
//
//  Created by Thomas Foster on 9/22/22.
//

#include "vector.h"
#include "mathlib.h"

extern inline vec2_t Vec2Add(vec2_t a, vec2_t b);
extern inline vec2_t Vec2Scale(vec2_t v, float s);
extern inline vec2_t Vec2Subtract(vec2_t a, vec2_t b);
extern inline float Vec2LengthSqr(vec2_t v);
extern inline float Vec2Length(vec2_t);
extern inline float Vec2Angle(vec2_t v);

void Vec2Lerp(vec2_t * v, const vec2_t * target, float w)
{
    v->x = Lerp(v->x, target->x, w);
    v->y = Lerp(v->y, target->y, w);
}

void Vec3Lerp(vec3_t * v, const vec3_t * target, float w)
{
    v->x = Lerp(v->x, target->x, w);
    v->y = Lerp(v->y, target->y, w);
    v->z = Lerp(v->z, target->z, w);
}

bool Vec2LerpEpsilon(vec2_t * v, vec2_t * target, float w, float epsilon)
{
    Vec2Lerp(v, target, w);

    float dx = fabsf(target->x - v->x);
    float dy = fabsf(target->y - v->y);

    if ( dx < epsilon && dy < epsilon ) {
        *v = *target;
        return true;
    }

    return false;
}

bool Vec3LerpEpsilon(vec3_t * v, vec3_t * target, float w, float epsilon)
{
    Vec3Lerp(v, target, w);

    float dx = fabsf(target->x - v->x);
    float dy = fabsf(target->y - v->y);
    float dz = fabsf(target->z - v->z);

    if ( dx < epsilon && dy < epsilon && dz < epsilon ) {
        *v = *target;
        return true;
    }

    return false;
}

vec2_t Vec2Normalize(vec2_t v)
{
    vec2_t result = { 0, 0 };

    float length = sqrtf(v.x * v.x + v.y * v.y);
    if ( length == 0.0f ) {
        return result;
    }

    float ilength = 1.0 / length;
    result.x = v.x * ilength;
    result.y = v.y * ilength;

    return result;
}

vec2_t Vec2Rotate(vec2_t v, float radians)
{
    vec2_t result;
    result.x = cos(radians) * v.x - sin(radians) * v.y;
    result.y = sin(radians) * v.x + cos(radians) * v.y;

    return result;
}
