#ifndef __RENDERER_C__
#define __RENDERER_C__


#include "internal.h"
#include "draw/shader.h"
#include "draw/buffer.h"
#include "draw/texture.h"
#include "vector.h"

typedef struct Mesh {
    uint32_t meshID;
    Shader shader;
    VertexBuffer vertBuff;
    IndexBuffer indexBuff;
    Texture texture;
} Mesh;

typedef struct Camera {
    float left, right;
    float bottom, top;
    float near, far;
    Mat4 m4Ortho;
} Camera;

void initRenderer();

void initMesh(
    Mesh* mesh, const char* vertPath, const char* fragPath,
    size_t vertCount, Vertice* vertices, size_t indCount, Indice* indices, uint32_t drawType,
    uint32_t slot, const char* texPath
);

void updateMeshBuffers(Mesh* mesh, size_t vertCount, Vertice* vertices, size_t indCount, Indice* indices, uint32_t drawType);

void drawMesh(Mesh* mesh, const Camera* camera);

void deleteMesh(Mesh* mesh);

Camera getCamera(float left, float right, float bottom, float top, float near, float far);

void moveCamera(Camera* camera, Vector pos);

void makeTextData(const char* text, const uint32_t len, Vector pos, Vector color, const float size, Vertice* glyphs);
void drawText(const char* text, const uint32_t len, const Vector pos, const Vector color,const float size);

void deleteRenderer();

static void getGlyphPosition(char glyph, uint32_t* X, uint32_t* Y);

#endif // __RENDERER_C__