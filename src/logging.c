#include "logging.h"

void clearError() {
    while (glGetError() != GL_NO_ERROR);
}

uint32_t checError(const char* func) {
    uint32_t error;
    while (error = glGetError()) {
        printf("[ERROR] %X | Function: %s\n", error, func);
        return 0;
    }
    return 1;
}