//
//  vector.h
//  Game
//
//  Created by Thomas Foster on 9/22/22.
//

#ifndef vector_h
#define vector_h

#include <stdbool.h>
#include <math.h>

typedef struct {
    float x;
    float y;
} vec2_t;

typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

vec2_t Vec2Normalize(vec2_t v);
vec2_t Vec2Rotate(vec2_t v, float radians);
void Vec2Lerp(vec2_t * v, const vec2_t * target, float w);
void Vec3Lerp(vec3_t * v, const vec3_t * target, float w);
bool Vec2LerpEpsilon(vec2_t * v, vec2_t * target, float w, float epsilon);
bool Vec3LerpEpsilon(vec3_t * v, vec3_t * target, float w, float epsilon);

#define VectorLerp(v, t, w) _Generic((v), \
    vec2_t *: Vec2Lerp, \
    vec3_t *: Vec3Lerp \
)(v, t, w)

#define VectorLerpEpsilon(v, t, w, e) _Generic((v), \
    vec2_t *: Vec2LerpEpsilon, \
    vec3_t *: Vec3LerpEpsilon \
)(v, t, w, e)

inline vec2_t Vec2Add(vec2_t a, vec2_t b)
{
    return (vec2_t){ a.x + b.x, a.y + b.y };
}

inline vec2_t Vec2Subtract(vec2_t a, vec2_t b)
{
    return (vec2_t){ a.x - b.x, a.y - b.y };
}

inline vec2_t Vec2Scale(vec2_t v, float s)
{
    return (vec2_t){ v.x * s, v.y * s };
}

inline float Vec2LengthSqr(vec2_t v)
{
    return v.x * v.x + v.y * v.y;
}

inline float Vec2Length(vec2_t v)
{
    return sqrtf(Vec2LengthSqr(v));
}

inline float Vec2Angle(vec2_t v)
{
    return atan2f(-v.y, v.x);
}

#endif /* vector_h */
