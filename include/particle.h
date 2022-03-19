#ifndef __PRARTICLE_H__
#define __PRARTICLE_H__

#include "internal.h"
#include "renderer.h"
#include "vector.h"

typedef struct Node {
    uint8_t isMutable;
    int32_t indexSet;
    int32_t indexHeap;
    float distance;
    Vector flow;
    struct Node* neighbours[4];
} Node;
static inline int32_t getIndex(int32_t x, int32_t y);

typedef struct Particle {
    Vector position;
    Vector velocity;
} Particle;

void initBoard();
void particleKeysCallback(int32_t key, int32_t scancode, int32_t action, int32_t mod);

void addRemoveWalls(const double xM, const double yM);

void destroyBoard();

static void findNeighbours(Node* pos, const int32_t x, const int32_t y);

static void flowField(const int32_t xInd, const int32_t yInd);

// #define HEAP
#ifdef HEAP
static void heapPush(Node* heap[], Node* item, int32_t* count);
static Node* remFirst(Node* heap[], int32_t* count);
static void sortUp(Node* heap[], Node* item);
static void sortDown(Node* heap[], Node* item, const int32_t* count);
static void swapNodes(Node* heap[], Node* a, Node* b);
#else
static Node* findSmallestNode(Node* list[]);
#endif // HEAP_DEBUG

static void displayBoard();

static void updateParticles(const int32_t BoardRootX, const int32_t BoardRootY);

void updateBoard(GLFWwindow* window);

#endif // __PRARTICLE_H__