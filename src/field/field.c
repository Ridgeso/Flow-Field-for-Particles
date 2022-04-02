#include <field/field.h>

Node Board[BOARD_X * BOARD_Y];

void findNeighbours(Node* pos, const int32_t x, const int32_t y) {
    int32_t counter = 0;
    for (int32_t j = -1; j < 2; ++j) {
        for (int32_t i= -1; i < 2; ++i) {
            if ((i == 0 && j == 0) || i == j  || -i == j) continue;
            int32_t iX = x+i, iY = y+j;
            if (0 <= iX && iX < BOARD_X && 0 <= iY && iY < BOARD_Y)
                pos->neighbours[counter] = &Board[getIndex(iX, iY)];
            else
                pos->neighbours[counter] = NULL;
            ++counter;
        }
    }
}

void flowField(const int32_t xInd, const int32_t yInd) {
    // Generate distance map
    Node* pos = &Board[getIndex(xInd, yInd)];
    pos->distance = 0;
#ifdef HEAP
    int32_t currentCount = 0;
    Node* openSet[BOARD_X*BOARD_Y];
#else
    Node* openSet[BOARD_X*BOARD_Y] = {NULL};
#endif
    int8_t closedSet[BOARD_X*BOARD_Y] = {0};
    closedSet[pos->indexSet] = 1;

#ifdef HEAP
    for(Node* current = pos; current != NULL; current = remFirst(openSet, &currentCount)) {
#else 
    for(Node* current = pos; current != NULL; current = findSmallestNode(openSet)) {
#endif
        for (int32_t j = 0; j < 4; ++j) {
            Node* neighbour = current->neighbours[j];
            if (neighbour == NULL || closedSet[neighbour->indexSet] || !neighbour->isMutable) continue;
            
            closedSet[neighbour->indexSet] = 1;
            neighbour->distance = current->distance+1;
#ifdef HEAP
            heapPush(openSet, neighbour, &currentCount);
#else
            openSet[neighbour->indexSet] = neighbour;
#endif
        }
    }

    // GENERATE FLOW VECTORS FOR EACH FIELD
    // set distance for bounders
    // left and right without corners
    for (int32_t j = 1; j < BOARD_Y-1; ++j) {
        Board[getIndex(0, j)].distance = Board[getIndex(1, j)].distance+1;
        Board[getIndex(BOARD_X-1, j)].distance = Board[getIndex(BOARD_X-2, j)].distance+1;
    }
    // top and botto with corners
    for(int32_t i = 0; i < BOARD_X; ++i) {
        Board[getIndex(i, 0)].distance = Board[getIndex(i, 1)].distance+1;
        Board[getIndex(i, BOARD_Y-1)].distance = Board[getIndex(i, BOARD_Y-2)].distance+1;
    }

    // update flow vectors
    for (int32_t j = 1; j < BOARD_Y-1; ++j) {
        for (int32_t i = 1; i < BOARD_X-1; ++i) {
            Node* current = &Board[getIndex(i, j)];
            if (!current->isMutable) continue;

            if(!current->neighbours[0]->isMutable && j != 1)
                current->neighbours[0]->distance = current->distance+1;
            if(!current->neighbours[1]->isMutable && i != 1)
                current->neighbours[1]->distance = current->distance+1;
            if(!current->neighbours[2]->isMutable && i != BOARD_X-2)
                current->neighbours[2]->distance = current->distance+1;
            if(!current->neighbours[3]->isMutable && j != BOARD_Y-2)
                current->neighbours[3]->distance = current->distance+1;

            Vector f = vec2(
                current->neighbours[1]->distance - current->neighbours[2]->distance, // Right - Left
                current->neighbours[3]->distance - current->neighbours[0]->distance  // Down - Up
            );

            // checking if the field is between the same distances
            if (f.x == 0 && current->neighbours[1]->distance < current->distance && current->neighbours[2]->distance < current->distance)
                f.x = 1;
            if (f.y == 0 && current->neighbours[0]->distance < current->distance && current->neighbours[3]->distance < current->distance)
                f.y = 1;

            current->flow = normalized(f);
        }
    }
}

#ifdef HEAP
static void heapPush(Node* heap[], Node* item, int32_t* count) {
    item->indexHeap = *count;
    heap[*count] = item;
    sortUp(heap, item);
    ++(*count);
}

static Node* remFirst(Node* heap[], int32_t* count) {
    if (*count == 0) return NULL;
    Node* firstItem = heap[0];
    (*count)--;
    heap[0] = heap[*count];
    heap[0]->indexHeap = 0;
    sortDown(heap, heap[0], count);
    return firstItem;
}

static void sortUp(Node* heap[], Node* item) {
    int32_t parent;
    while ((parent = (item->indexHeap - 1) / 2) >= 0) {
        Node* parentItem = heap[parent];
        if (item->distance < parentItem->distance)
            swapNodes(heap, parentItem, item);
        else break;
    }
}

static void sortDown(Node* heap[], Node* item, const int32_t* count) {
    while (1) {
        int32_t leftChild = item->indexHeap*2+1;
        int32_t rightChild = item->indexHeap*2+2;
        if (leftChild < *count) {
            int32_t swapIndex = leftChild;
            if (rightChild < *count && heap[leftChild]->distance > heap[rightChild]->distance)
                swapIndex = rightChild;

            if (heap[swapIndex]->distance < item->distance)
                swapNodes(heap, item, heap[swapIndex]);
            else return;
        }
        else return;
    }
}

static void swapNodes(Node* heap[], Node* a, Node* b) {
    heap[a->indexHeap] = b;
    heap[b->indexHeap] = a;
    int temp = a->indexHeap;
    a->indexHeap = b->indexHeap;
    b->indexHeap = temp;
}
#else
static Node* findSmallestNode(Node* list[]) {
    Node* smallestNode = NULL;
    for (int32_t i = 0; i < BOARD_X*BOARD_Y; ++i) {
        if (list[i] == NULL) continue;
        if (smallestNode == NULL || list[i]->distance < smallestNode->distance)
            smallestNode = list[i];
    }
    if (smallestNode == NULL) return NULL;

    list[smallestNode->indexSet] = NULL;
    return smallestNode;
}
#endif