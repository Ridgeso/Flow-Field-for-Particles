#include "draw/buffer.h"

void setVertexBufferData(VertexBuffer* vertex, size_t count, Vertice* vertices, uint32_t drawMode) {
    vertex->vertCount = count;
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertex->renderID));
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertex->vertCount*sizeof(Vertice), (void*)vertices, drawMode));
}

void addLayout(uint32_t layout, uint32_t numOfComponets, size_t offset) {
    GLCall(glEnableVertexAttribArray(layout));
    GLCall(glVertexAttribPointer(layout, numOfComponets, GL_FLOAT, GL_FALSE, sizeof(Vertice), (void*)(offset)));
}

void setIndexBufferData(IndexBuffer* index, size_t count, Indice* indices, uint32_t drawMode) {
    index->indCount = count;
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index->renderID));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, index->indCount*sizeof(Indice), (void*)indices, drawMode));
}