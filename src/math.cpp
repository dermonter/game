typedef double real64_t;
typedef float real32_t;
typedef int bool32_t;
#define PI32 3.14159265359f
#include <stdint.h>
#include <math.h>

struct vec3 {
    union {
        struct {
            int32_t x;
            int32_t y;
            int32_t z;
        };
        int32_t values[3];
    };
};

struct vec3f {
    union {
        struct {
            real32_t x;
            real32_t y;
            real32_t z;
        };
        real32_t values[3];
    };
};

vec3f operator*(real32_t n, vec3f v) {
    vec3f result = {v.x * n, v.y * n, v.z * n};
    return result;
}

vec3f operator+(vec3f a, vec3f b) {
    vec3f result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}

real32_t interpolate(real32_t a, real32_t b, real32_t t) {
    return t * a + (1.0f - t) * b;
}

vec3f interpolate(vec3f a, vec3f b, real32_t t) {
    return t * a + (1.0f - t) * b;
}