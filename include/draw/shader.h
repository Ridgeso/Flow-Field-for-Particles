#ifndef __SHADER_H__
#define __SHADER_H__

#include "internal.h"
#include <stdlib.h>

#define MAX_SHADER_PATH_LENGHT 32
typedef struct Shader {
    uint32_t shaderID;
    char vertFile[MAX_SHADER_PATH_LENGHT];
    char fragFile[MAX_SHADER_PATH_LENGHT];
} Shader;

typedef enum shaderType {PROGRAM, VERTEX, FRAGMENT} shaderType;

void initShader(Shader* shader, const char* vertPath, const char* fragPath);

#endif // __SHADER_H__