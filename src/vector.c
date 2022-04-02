#include "vector.h"
#include <math.h>
#include <stdint.h>

Vector vec() {
#if defined(SIMD_DEFINITION)
    return (Vector)_mm_setzero_ps();
#else
    return (Vector){{0.0, 0.0, 0.0, 0.0}};
#endif
}

Vector vec2(float x, float y) {
    return (Vector){{x, y, 0.0, 0.0}};
}

Vector vec3(float x, float y, float z) {
    return (Vector){{x, y, z, 0.0}};
}

Vector vec4(float x, float y, float z, float w) {
    return (Vector){{x, y, z, w}};
}

float dot(Vector a, Vector b) {
#if defined(SIMD_DEFINITION)
    const __m128 _mul = _mm_mul_ps(a._vec, b._vec);

    const __m128 _sum = _mm_hadd_ps(_mul, _mul);
    const __m128 _dot = _mm_hadd_ps(_sum, _sum);

    return _mm_cvtss_f32(_dot);
#else
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
#endif
};

float magnitude(Vector v) {
    return sqrtf(dot(v, v));
}

static float Q_rsqrt(float number) {
    float x2 = number * 0.5F;
    const float threehalfs = 1.5F;

    union {
        float f;
        uint32_t i;
    } conv = {number};

    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= threehalfs - (x2 * conv.f * conv.f);

    return conv.f;
}

Vector normalized(Vector v) {
    const float norm = Q_rsqrt(dot(v, v));
#if defined(SIMD_DEFINITION)
    const __m128 _norm = _mm_set1_ps(norm);
    const __m128 _n = _mm_mul_ps(v._vec, _norm);
    return (Vector)_n;
#else
    return (Vector){{v.x * norm, v.y * norm, v.z * norm, v.w * norm}};
#endif
}

Vector addV(Vector a, Vector b) {
#if defined(SIMD_DEFINITION)
    const __m128 _sum = _mm_add_ps(a._vec, b._vec);
    return (Vector)_sum;

#else
    return (Vector){{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}};
#endif
}

Vector subV(Vector a, Vector b) {
#if defined(SIMD_DEFINITION)
    const __m128 _sub = _mm_sub_ps(a._vec, b._vec);
    return (Vector)_sub;
#else
    return (Vector){{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}};
#endif
}

Vector mulSV(Vector a, float s) {
#if defined(SIMD_DEFINITION)
    const __m128 _s = _mm_set1_ps(s);
    const __m128 _mul = _mm_mul_ps(a._vec, _s);
    return (Vector)_mul;
#else
    return (Vector){{a.x * s, a.y * s, a.z * s, a.w * s}};
#endif
}

Vector cross(Vector a, Vector b) {
#if defined(SIMD_DEFINITION)
    const __m128 _shuffle0 = _mm_shuffle_ps(a._vec, a._vec, _MM_SHUFFLE(3, 0, 2, 1));
    const __m128 _shuffle1 = _mm_shuffle_ps(b._vec, b._vec, _MM_SHUFFLE(3, 1, 0, 2));
    
    const __m128 _shuffle2 = _mm_shuffle_ps(a._vec, a._vec, _MM_SHUFFLE(3, 1, 0, 2));
    const __m128 _shuffle3 = _mm_shuffle_ps(b._vec, b._vec, _MM_SHUFFLE(3, 0, 2, 1));

    
    const __m128 _mul0 = _mm_mul_ps(_shuffle0, _shuffle1);
    const __m128 _mul1 = _mm_mul_ps(_shuffle2, _shuffle3);

    const __m128 _cross = _mm_sub_ps(_mul0, _mul1);
    return (Vector)_cross;
#else
    return (Vector){{a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x, 0.0}};
#endif
}

Mat4 mat() {
    const Mat4 mat = {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }};
    return mat;
}

Mat4 mat2(float x, float y) {
    const Mat4 mat = {{
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }};
    return mat;
}

Mat4 mat3(float x, float y, float z) {
    const Mat4 mat = {{
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    }};
    return mat;
}

Mat4 mat4(float x, float y, float z, float w) {
    const Mat4 mat = {{
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, w
    }};
    return mat;
}

Mat4 mulSM(Mat4* a, float s) {
    Mat4 scalesMat;
#if defined(SIMD_DEFINITION)
    __m128 _s = _mm_set1_ps(s);
    scalesMat._vec[0] = _mm_mul_ps(a->_vec[0], _s);
    scalesMat._vec[1] = _mm_mul_ps(a->_vec[1], _s);
    scalesMat._vec[2] = _mm_mul_ps(a->_vec[2], _s);
    scalesMat._vec[3] = _mm_mul_ps(a->_vec[3], _s);
#else
    for (int32_t i = 0; i < 16; ++i)
        scalesMat.values[i] = a->values[i] * s;
#endif
    return scalesMat;
}

Vector mulMV(Mat4* a, Vector b) {
#if defined(SIMD_DEFINITION)
    const __m128 _v0 = _mm_mul_ps(a->_vec[0], _mm_set1_ps(b.values[0]));
    const __m128 _v1 = _mm_mul_ps(a->_vec[1], _mm_set1_ps(b.values[1]));
    const __m128 _v2 = _mm_mul_ps(a->_vec[2], _mm_set1_ps(b.values[2]));
    const __m128 _v3 = _mm_mul_ps(a->_vec[3], _mm_set1_ps(b.values[3]));

    const __m128 _add0 = _mm_add_ps(_v0, _v1);
    const __m128 _add1 = _mm_add_ps(_add0, _v2);
    const __m128 _add2 = _mm_add_ps(_add1, _v3);

    return (Vector)_add2;
#else
    Vector multipliedVec = vec();
    for (int32_t i = 0; i < 4; ++i)
        for (int32_t j = 0; j < 4; ++j)
            multipliedVec.values[i] += a->values[i*4+j] * b.values[j];
    return multipliedVec;
#endif
}

Mat4 mulMM(Mat4* a, Mat4* b) {
    Mat4 multipliedMat;
#if defined(SIMD_DEFINITION)
    for (int32_t i = 0; i < 4; ++i) {
        const __m128 _r0 = _mm_mul_ps(_mm_set1_ps(a->vec[i].x), b->_vec[0]);
        const __m128 _r1 = _mm_mul_ps(_mm_set1_ps(a->vec[i].y), b->_vec[1]);
        const __m128 _r2 = _mm_mul_ps(_mm_set1_ps(a->vec[i].z), b->_vec[2]);
        const __m128 _r3 = _mm_mul_ps(_mm_set1_ps(a->vec[i].w), b->_vec[3]);

        const __m128 _sum0 = _mm_add_ps(_r0, _r1);
        const __m128 _sum1 = _mm_add_ps(_r2, _r3);
        multipliedMat._vec[i] = _mm_add_ps(_sum0, _sum1);
    }
#else
    for (int32_t i = 0; i < 4; ++i) {
        Vector column = vec4(b->vx[i], b->vy[i], b->vz[i], b->vw[i]);
        for (int32_t j = 0; j < 4; ++j) {
            multipliedMat.values[i*4+j] = dot(a->vec[j], column);
        }
    }
#endif
    return multipliedMat;
}

Vector rotate(Vector v, float angle) {
    const float c = cosf(angle);
    const float s = sinf(angle);
    Mat4 rotation = (Mat4){{
        c, -s, 0, 0,
        s,  c, 0, 0,
        0,  0, 1, 0,
        0,  0, 0, 1
    }};
    return mulMV(&rotation, v);
}