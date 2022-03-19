#include "renderer.h"
#include <math.h>

#define PixelWidth  5.0
#define PixelHeight 7.0
static Mesh Font;
void initRenderer() {
    initMesh(&Font, "../shaders/font.vert", "../shaders/font.frag", 0, NULL, 0, NULL, GL_DYNAMIC_DRAW, 0, "../shaders/assets/font.png");
}

void initMesh(
    Mesh* mesh, const char* vertPath, const char* fragPath,
    size_t vertCount, Vertice* vertices, size_t indCount, Indice* indices, uint32_t drawType,
    uint32_t slot, const char* texPath
) {
    // BEGIN :: SHADERS
    initShader(&mesh->shader, vertPath, fragPath);
    // END :: SHADERS

    // BEGIN :: Vertex and Index buffers
    GLCall(glGenVertexArrays(1, &mesh->meshID));
    GLCall(glBindVertexArray(mesh->meshID));


    GLCall(glGenBuffers(1, &mesh->vertBuff.renderID));
    setVertexBufferData(&mesh->vertBuff, vertCount, vertices, drawType);
    
    addLayout(0, 3, offsetof(Vertice, pos));
    addLayout(1, 4, offsetof(Vertice, color));
    addLayout(2, 2, offsetof(Vertice, tx));


    GLCall(glGenBuffers(1, &mesh->indexBuff.renderID));
    setIndexBufferData(&mesh->indexBuff, indCount, indices, drawType);


    GLCall(glBindVertexArray(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    // END :: Vertex and Index buffers

    // END :: Texture
    if (texPath != NULL) {
        loadTexture(&mesh->texture, texPath);
        initTexture(&mesh->texture, slot, GL_NEAREST, GL_CLAMP_TO_EDGE);
    }
    // END :: Texture
}

void updateMeshBuffers(Mesh* mesh, size_t vertCount, Vertice* vertices, size_t indCount, Indice* indices, uint32_t drawType) {
    GLCall(glBindVertexArray(mesh->meshID));
    
    setVertexBufferData(&mesh->vertBuff, vertCount, vertices, drawType);
    setIndexBufferData(&mesh->indexBuff, indCount, indices, drawType);
    
    GLCall(glBindVertexArray(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

// TODO: Make draw function for each component
void drawMesh(Mesh* mesh, const Camera* camera) {
    GLCall(glBindVertexArray(mesh->meshID));
    GLCall(glUseProgram(mesh->shader.shaderID));

    GLCall(glActiveTexture(GL_TEXTURE0+mesh->texture.slot));
    GLCall(glBindTexture(GL_TEXTURE_2D, mesh->texture.texID));

    GLCall(glUniform1i(glGetUniformLocation(mesh->shader.shaderID, "tex"), mesh->texture.slot));
    
    GLCall(glUniformMatrix4fv(glGetUniformLocation(mesh->shader.shaderID, "ortho"), 1, GL_FALSE, camera->m4Ortho.values));
    GLCall(glDrawElements(GL_TRIANGLES, mesh->indexBuff.indCount*3, GL_UNSIGNED_INT, NULL));

    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
    GLCall(glUseProgram(0));
    GLCall(glBindVertexArray(0));
}

void deleteMesh(Mesh* mesh) {
    // Shaders    
    GLCall(glDeleteProgram(mesh->shader.shaderID));

    // Buffers
    GLCall(glDeleteBuffers(1, &mesh->meshID));
    GLCall(glDeleteBuffers(1, &mesh->vertBuff.renderID));
    GLCall(glDeleteBuffers(1, &mesh->indexBuff.renderID));

    // Textures
    freeTextureData(&mesh->texture);
    GLCall(glDeleteTextures(1, &mesh->texture.texID));
}

Camera getCamera(float left, float right, float bottom, float top, float near, float far) {
    Camera camera = (Camera){left, right, bottom, top, near, far, mat()};
    // Diagonal
    camera.m4Ortho.x.x = 2/(right - left);
    camera.m4Ortho.y.y = 2/(top - bottom);
    camera.m4Ortho.z.z = 2/(far - near);
    
    // W
    camera.m4Ortho.w.x = -(right+left)/(right-left);
    camera.m4Ortho.w.y = -(top+bottom)/(top-bottom);
    camera.m4Ortho.w.z = -(far+near)/(far-near);

    return camera;
}

void moveCamera(Camera* camera, Vector pos) {
    camera->m4Ortho.w.x = pos.x;
    camera->m4Ortho.w.y = pos.y;
    camera->m4Ortho.w.z = pos.z;
}

void makeTextData(const char* text, const uint32_t len, Vector pos, Vector color, const float size, Vertice* glyphs) {
    static const float padding = 0.0005;
    static const float fRatio = PixelHeight/PixelWidth;
    const float fWidth = size / WINDOW_WIDTH, fHeight = fRatio * size / WINDOW_HEIGHT;
    const float fSpriteWidth = PixelWidth / (float)Font.texture.width, fSpriteHeight = PixelHeight / (float)Font.texture.height;

    const char* c = text;
    for(int32_t glyph = 0; glyph < len; ++glyph) {
        uint32_t glyphPosX, glyphPosY;
        getGlyphPosition(*c, &glyphPosX, &glyphPosY);
        //                                          POS                                         COLOR             Tex X                       Tex Y
        glyphs[4*glyph+0] = (Vertice){vec2(pos.x+glyph*(fWidth+padding),        pos.y),         color, (glyphPosX+0)*fSpriteWidth, (glyphPosY+1)*fSpriteHeight};
        glyphs[4*glyph+1] = (Vertice){vec2(pos.x+fWidth+glyph*(fWidth+padding), pos.y),         color, (glyphPosX+1)*fSpriteWidth, (glyphPosY+1)*fSpriteHeight};
        glyphs[4*glyph+2] = (Vertice){vec2(pos.x+fWidth+glyph*(fWidth+padding), pos.y-fHeight), color, (glyphPosX+1)*fSpriteWidth, (glyphPosY+0)*fSpriteHeight};
        glyphs[4*glyph+3] = (Vertice){vec2(pos.x+glyph*(fWidth+padding),        pos.y-fHeight), color, (glyphPosX+0)*fSpriteWidth, (glyphPosY+0)*fSpriteHeight};

        ++c;
    }
}

void drawText(const char* text, const uint32_t len, const Vector pos, const Vector color, const float size) {

    {
        Vertice glyphs[4*len];
        makeTextData(text, len, pos, color, size, glyphs);

        Indice glyphCount[2*len];
        int32_t count = 0;
        for(int32_t ind = 0; ind < 2*len; ind += 2) {
            glyphCount[ind + 0] = (Indice){count+0, count+1, count+2};
            glyphCount[ind + 1] = (Indice){count+2, count+3, count+0};
            count += 4;
        }

        updateMeshBuffers(&Font, 4*len, glyphs, 2*len, glyphCount, GL_DYNAMIC_DRAW);
    }

    Camera fontCamera = getCamera(-1, 1, -1, 1, 0, -1);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawMesh(&Font, &fontCamera);
    glBlendFunc (GL_ONE, GL_ZERO);
}

void deleteRenderer() {
    deleteMesh(&Font);
}

static void getGlyphPosition(char glyph, uint32_t* X, uint32_t* Y) {
    static const uint32_t charactersInRow = 18, charactersInColumn = 6;
    glyph -= ' ';
    
    *X = glyph % charactersInRow;
    
    // [0, 0] = 'z' because image origin is in bottom right corner, so Y index must be flipped
    *Y = charactersInColumn - 1 - glyph / charactersInRow;
    
    ASSERT(*X < 0 || charactersInRow <= *X, GLYPH OUT OF SCOPE)
    ASSERT(*Y < 0 || charactersInColumn <= *Y, GLYPH OUT OF SCOPE)
}