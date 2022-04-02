#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "internal.h"
#include "vector.h"

typedef struct Vertice {
    Vector pos;
    Vector color;
    float tx, ty;
} Vertice;


typedef struct VertexBuffer {
    uint32_t renderID;
    size_t vertCount;
} VertexBuffer;

void setVertexBufferData(VertexBuffer* vertex, size_t count, Vertice* vertices, uint32_t drawMode);

void addLayout(uint32_t layout, uint32_t numOfComponets, size_t offset);

typedef struct Indice {
    uint32_t x, y, z;
} Indice;


typedef struct IndexBuffer {
    uint32_t renderID;
    size_t indCount;
} IndexBuffer;

void setIndexBufferData(IndexBuffer* index, size_t count, Indice* indices, uint32_t drawMode);

#endif // __BUFFER_H__