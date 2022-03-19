#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "internal.h"
#include "stb/stb_image.h"

typedef struct Texture {
    uint32_t texID;
    int32_t width, height, bytesPerPixel;
    uint32_t slot;
    uint8_t* data;
} Texture;

void loadTexture(Texture* tex, const char* path);

void initTexture(Texture* tex, uint32_t slot, uint32_t scaleOption, uint32_t wrapOption);

void freeTextureData(Texture* tex);


#endif // __TEXTURE_H__