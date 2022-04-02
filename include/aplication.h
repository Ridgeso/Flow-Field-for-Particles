#ifndef __APLICATION_H__
#define __APLICATION_H__

#include <string.h>
#include "internal.h"
#include "particle.h"
#include "renderer.h"

int32_t initAplication();

void keysCallBack(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mod);
void mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods);

void mainLoop();

typedef struct Deltatime {
    double prevTime;
    double currTime;
    double timeDiff;
    double frameCounter;
} Deltatime;

#ifdef DEBUG
void updateFrameCounter();
#else
void drawFrameCounter();
#endif // DEBUG

void destroyAplication();

#endif // __APLICATION_H__