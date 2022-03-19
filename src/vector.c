#include "vector.h"
#include <math.h>
#include <stdint.h>

Vector vec() {
    return (Vector){{0.0, 0.0, 0.0, 0.0}};
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

float magnitude(Vector v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
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
    float norma = Q_rsqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    return (Vector){{v.x * norma, v.y * norma, v.z * norma, v.w * norma}};
}

float dot(Vector a, Vector b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
};

Vector addV(Vector a, Vector b) {
    return (Vector){{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w}};
}

Vector subV(Vector a, Vector b) {
    return (Vector){{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w}};
}

Vector mulSV(Vector a, float s) {
    return (Vector){{a.x * s, a.y * s, a.z * s, a.w * s}};
}

Vector cross(Vector a, Vector b) {
    return (Vector){{a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x, 0.0}};
}

Mat4 mat() {
    Mat4 mat;
    for (int32_t i = 0; i < 4; ++i)
        for (int32_t j = 0; j < 4; ++j)
            mat.values[i*4+j] = i == j ? 1 : 0;
    return mat;
}

Mat4 mat2(float x, float y) {
    Mat4 mat;
    for (int32_t i = 0; i < 4; ++i)
        for (int32_t j = 0; j < 4; ++j)
            mat.values[i*4+j] = 0;
    mat.x.x = x;
    mat.y.y = y;
    return mat;
}

Mat4 mat3(float x, float y, float z) {
    Mat4 mat;
    for (int32_t i = 0; i < 4; ++i)
        for (int32_t j = 0; j < 4; ++j)
            mat.values[i*4+j] = 0;
    mat.x.x = x;
    mat.y.y = y;
    mat.z.z = z;
    return mat;
}

Mat4 mat4(float x, float y, float z, float w) {
    Mat4 mat;
    for (int32_t i = 0; i < 4; ++i)
        for (int32_t j = 0; j < 4; ++j)
            mat.values[i*4+j] = 0;
    mat.x.x = x;
    mat.y.y = y;
    mat.z.z = z;
    mat.w.w = w;
    return mat;
}

Mat4 mulSM(Mat4* a, float s) {
    Mat4 scalesMat;
    for (int32_t i = 0; i < 16; ++i)
        scalesMat.values[i] = a->values[i] * s;
    return scalesMat;
}

Vector mulMV(Mat4* a, Vector b) {
    Vector multipliedVec = vec();
    for (int32_t i = 0; i < 4; ++i)
        for (int32_t j = 0; j < 4; ++j)
            multipliedVec.values[i] += a->values[i*4+j] * b.values[j];
    return multipliedVec;
}

Mat4 mulMM(Mat4* a, Mat4* b) {
    Mat4 multipliedMat;
    for (int32_t i = 0; i < 4; ++i) {
        Vector column = vec4(b->vx[i], b->vy[i], b->vz[i], b->vw[i]);
        for (int32_t j = 0; j < 4; ++j) {
            multipliedMat.values[i*4+j] = dot(a->vec[j], column);
        }
    }
    return multipliedMat;
}

Vector rotate(Vector v, float angle) {
    Mat4 rotation = (Mat4){{
        cosf(angle), -sinf(angle), 0, 0,
        sinf(angle),  cosf(angle), 0, 0,
        0,            0,           1, 0,
        0,            0,           0, 1
    }};
    return mulMV(&rotation, v);
}