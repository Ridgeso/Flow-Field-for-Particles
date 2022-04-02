#ifndef __VECTOR_H__
#define __VECTOR_H__


#if defined(SIMD_DEFINITION)
    #include <immintrin.h>
#endif

typedef union Vector {
    float values[4];
#if defined(SIMD_DEFINITION)
    __m128 _vec;
#endif
    struct {
        float x, y, z, w;
    };
    struct {
        float r, b, g, a;
    };
} Vector;

Vector vec();
Vector vec2(float x, float y);
Vector vec3(float x, float y, float z);
Vector vec4(float x, float y, float z, float w);

float dot(Vector a, Vector b);
float magnitude(Vector v);
Vector normalized(Vector n);

Vector addV(Vector a, Vector b);
Vector subV(Vector a, Vector b);
Vector mulSV(Vector n, float s);
Vector cross(Vector a, Vector b);

typedef union Mat4 {
    float values[16];
#if defined(SIMD_DEFINITION)
    __m128 _vec[4];
#endif
    struct {
        Vector x, y, z, w;
    };
    struct {
        Vector vec[4];
    };
    struct {
        float vx[4], vy[4], vz[4], vw[4];
    };
} Mat4;

Mat4 mat();
Mat4 mat2(float x, float y);
Mat4 mat3(float x, float y, float z);
Mat4 mat4(float x, float y, float z, float w);
Mat4 mulSM(Mat4* a, float s);
Vector mulMV(Mat4* a, Vector b);
Mat4 mulMM(Mat4* a, Mat4* b);

Vector rotate(Vector v, float angle);

#endif // __VECTOR_H__