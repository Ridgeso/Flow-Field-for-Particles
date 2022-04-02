#ifndef __PRARTICLE_H__
#define __PRARTICLE_H__

#include "internal.h"
#include "renderer.h"
#include "vector.h"

typedef struct Particle {
    Vector position;
    Vector velocity;
} Particle;

void initBoard();
void particleKeysCallback(int32_t key, int32_t scancode, int32_t action, int32_t mod);

void addRemoveWalls(const double xM, const double yM);

void destroyBoard();

void updateBoard(GLFWwindow* window);

static void displayBoard();
static void drawGridDistance();
static void drawGridArrow();

static void updateParticles(const int32_t BoardRootX, const int32_t BoardRootY);

#endif // __PRARTICLE_H__