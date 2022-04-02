#ifndef __FIELD_C__
#define __FIELD_C__


#include <internal.h>
#include "vector.h"

typedef struct Node {
    uint8_t isMutable;
    int32_t indexSet;
    int32_t indexHeap;
    float distance;
    Vector flow;
    struct Node* neighbours[4];
} Node;

static inline int32_t getIndex(int32_t x, int32_t y) { return y*BOARD_X+x; }

void findNeighbours(Node* pos, const int32_t x, const int32_t y);

void flowField(const int32_t xInd, const int32_t yInd);

#ifdef HEAP
static void heapPush(Node* heap[], Node* item, int32_t* count);
static Node* remFirst(Node* heap[], int32_t* count);
static void sortUp(Node* heap[], Node* item);
static void sortDown(Node* heap[], Node* item, const int32_t* count);
static void swapNodes(Node* heap[], Node* a, Node* b);
#else
static Node* findSmallestNode(Node* list[]);
#endif // HEAP_DEBUG

#endif // __FIELD_C__