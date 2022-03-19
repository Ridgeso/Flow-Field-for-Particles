#include "draw/texture.h"

void loadTexture(Texture* tex, const char* path) {
    stbi_set_flip_vertically_on_load(1);
    
    printf("OPENING TEXTURE: %s\n\n", path);
    tex->data = stbi_load(path, &tex->width, &tex->height, &tex->bytesPerPixel, 4);
    ASSERT(tex->data == NULL, IMAGE FILE NOT OPENED)
}

void freeTextureData(Texture* tex) {
    stbi_image_free(tex->data);
}

void initTexture(Texture* tex, uint32_t slot, uint32_t scaleOption, uint32_t wrapOption) {
    GLCall(glActiveTexture(GL_TEXTURE0+slot));
    GLCall(glGenTextures(1, &tex->texID));
    GLCall(glBindTexture(GL_TEXTURE_2D, tex->texID));
    tex->slot = slot;

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scaleOption));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scaleOption));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapOption));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapOption));

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data));

    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}