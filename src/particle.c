#include "particle.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

static Camera WorldCamera;
static int8_t Simulation = 1;
static int8_t DrawDistance = 1;

static Node Board[BOARD_X * BOARD_Y];
static inline int32_t getIndex(int32_t x, int32_t y) { return y*BOARD_X+x; }

static Mesh GridMesh;
static Mesh WallsMesh;
static Mesh EdgeMesh;

#define MAX_PARTICLES 50000
static int32_t ParticleCount = 10000;
static Particle Points[MAX_PARTICLES];
static Mesh ParticleMesh;
static float ParticleSize = 3;

void initBoard() {
    WorldCamera = getCamera(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, 0, -1);

    for (int32_t j = 0; j < BOARD_Y; ++j) {
        for (int32_t i = 0; i < BOARD_X; ++i) {
            int8_t isMutable = (i == 0 || j == 0 || i == BOARD_X-1 || j == BOARD_Y-1) ? 0 : 1;
            Board[getIndex(i, j)] = (Node){isMutable, getIndex(i, j), 0, 0.0, vec2(0, 0), {}};
            findNeighbours(&Board[getIndex(i, j)], i, j);
        }
    }
    
    initMesh(&GridMesh, "../shaders/font.vert", "../shaders/font.frag", 0, NULL, 0, NULL, GL_DYNAMIC_DRAW, 0, "../shaders/assets/font.png");
    {
        Vertice edges[] = {
            {vec2(0.0,                               0.0),              vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(WINDOW_WIDTH,                      0.0),              vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(WINDOW_WIDTH,            WINDOW_WIDTH ),              vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(0.0,                     WINDOW_HEIGHT),              vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},

            {vec2(GRID_WIDTH,              GRID_HEIGHT),                vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(WINDOW_WIDTH-GRID_WIDTH, GRID_HEIGHT),                vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(WINDOW_WIDTH-GRID_WIDTH, WINDOW_HEIGHT-GRID_HEIGHT),  vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(GRID_WIDTH,              WINDOW_HEIGHT-GRID_HEIGHT),  vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7}
        };
        Indice eInd[] = {
            {0, 1, 5}, {0, 4, 5}, // top
            {2, 3, 6}, {3, 6, 7}, // bottom
            {0, 3, 7}, {0, 4, 7}, // left
            {1, 2, 5}, {2, 5, 6}  // right
        };
        initMesh(&EdgeMesh, "../shaders/edge.vert", "../shaders/edge.frag", 8, edges, 8, eInd, GL_STATIC_DRAW, GL_TEXTURE31, NULL);
        initMesh(&WallsMesh, "../shaders/edge.vert", "../shaders/edge.frag", 0, NULL, 0, NULL, GL_STATIC_DRAW, GL_TEXTURE31, NULL);
    }
    
    time_t los;
    srand(time(&los));
    for (int32_t i = 0; i < ParticleCount; i++)
        Points[i] = (Particle){
            vec2(   // Position
                GRID_WIDTH *2.0+(WINDOW_WIDTH -4.0*GRID_WIDTH )*rand()/RAND_MAX,
                GRID_HEIGHT*2.0+(WINDOW_HEIGHT-4.0*GRID_HEIGHT)*rand()/RAND_MAX
            ),
            vec2( // Velocity
                cosf(M_PI*2*rand()/RAND_MAX),
                sinf(M_PI*2*rand()/RAND_MAX)
            )
        };
    glPointSize(ParticleSize);
    initMesh(&ParticleMesh, "../shaders/particle.vert", "../shaders/particle.frag", 4*BOARD_X*BOARD_Y, NULL, 2*BOARD_X*BOARD_Y, NULL, GL_DYNAMIC_DRAW, GL_TEXTURE31, NULL);
}

void particleKeysCallback(int32_t key, int32_t scancode, int32_t action, int32_t mod) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        if (Simulation) Simulation = 0;
        else Simulation = 1;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        if (DrawDistance) DrawDistance = 0;
        else DrawDistance = 1;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        if (ParticleSize < 5.0) {
            ParticleSize += 1.;
            glPointSize(ParticleSize);
        }
    }
    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        if (ParticleSize > 1.0) {
            ParticleSize -= 1.0;
            glPointSize(ParticleSize);
        }
    }
    #define addedParticles 100
    if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_REPEAT) {
        if (ParticleCount < MAX_PARTICLES-addedParticles+1) {
            for (int32_t i = ParticleCount; i < ParticleCount + addedParticles; i++)
                Points[i] = (Particle){vec2(GRID_WIDTH*2.0+(WINDOW_WIDTH-4.0*GRID_WIDTH)*rand()/RAND_MAX, 2.0*GRID_HEIGHT+(WINDOW_HEIGHT-4.0*GRID_HEIGHT)*rand()/RAND_MAX), vec2(0, 0)};
            ParticleCount += addedParticles;
        }
    }
    if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_REPEAT) {
        if (ParticleCount >= addedParticles)
            ParticleCount -= addedParticles;
    }
}

void addRemoveWalls(const double xM, const double yM) {
    static Vertice Walls[4 * BOARD_X*BOARD_Y];
    static Indice WallsCount[2 * BOARD_X*BOARD_Y];

    int32_t xInd = (int32_t)xM, yInd = (int32_t)yM;
    xInd /= GRID_WIDTH;
    yInd /= GRID_HEIGHT;
    int32_t wallIndex = getIndex(xInd, yInd)*2;
    if (!Board[getIndex(xInd, yInd)].isMutable && 0 < xInd && xInd < BOARD_X-1 && 0 < yInd && yInd < BOARD_Y-1) {
        Board[getIndex(xInd, yInd)].isMutable = 1;

        Walls[wallIndex*2+0] = (Vertice){vec(), vec4(0.3, 0.7, 1.0, 1.0), 0, 0};
        Walls[wallIndex*2+1] = (Vertice){vec(), vec4(0.3, 0.7, 1.0, 1.0), 0, 0};
        Walls[wallIndex*2+2] = (Vertice){vec(), vec4(0.3, 0.7, 1.0, 1.0), 0, 0};
        Walls[wallIndex*2+3] = (Vertice){vec(), vec4(0.3, 0.7, 1.0, 1.0), 0, 0};
        WallsCount[wallIndex+0] = (Indice){0, 0, 0};
        WallsCount[wallIndex+1] = (Indice){0, 0, 0};
    }
    else {
        Board[getIndex(xInd, yInd)].isMutable = 0;

        Walls[wallIndex*2+0] = (Vertice){vec2(GRID_WIDTH*(float)xInd,            WINDOW_HEIGHT-GRID_HEIGHT*(float)yInd), vec4(0.3, 0.7, 1.0, 1.0), 0, 0};
        Walls[wallIndex*2+1] = (Vertice){vec2(GRID_WIDTH+GRID_WIDTH*(float)xInd, WINDOW_HEIGHT-GRID_HEIGHT*(float)yInd), vec4(0.3, 0.7, 1.0, 1.0), 0, 0};
        Walls[wallIndex*2+2] = (Vertice){vec2(GRID_WIDTH+GRID_WIDTH*(float)xInd, WINDOW_HEIGHT-GRID_HEIGHT-GRID_HEIGHT*(float)yInd), vec4(0.3, 0.7, 1.0, 1.0), 0, 0};
        Walls[wallIndex*2+3] = (Vertice){vec2(GRID_WIDTH*(float)xInd,            WINDOW_HEIGHT-GRID_HEIGHT-GRID_HEIGHT*(float)yInd), vec4(0.3, 0.7, 1.0, 1.0), 0, 0};

        WallsCount[wallIndex+0] = (Indice){wallIndex*2+0, wallIndex*2+1, wallIndex*2+2};
        WallsCount[wallIndex+1] = (Indice){wallIndex*2+2, wallIndex*2+3, wallIndex*2+0};

    }
    updateMeshBuffers(&WallsMesh, 4*BOARD_X*BOARD_Y, Walls, 2*BOARD_X*BOARD_Y, WallsCount, GL_DYNAMIC_DRAW);
}

void destroyBoard() {
    deleteMesh(&GridMesh);
    deleteMesh(&EdgeMesh);
    deleteMesh(&WallsMesh);
    deleteMesh(&ParticleMesh);
}

static void findNeighbours(Node* pos, const int32_t x, const int32_t y) {
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

static void flowField(const int32_t xInd, const int32_t yInd) {
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

static void displayBoard() {
    #define maxGridTextLength 4
    #define gridsAmount (BOARD_X*BOARD_Y - 2*(BOARD_X+BOARD_Y-2))
    static Vertice grids[4*gridsAmount * (maxGridTextLength-1)];
    static Indice gridsCount[2*gridsAmount * (maxGridTextLength-1)];
    int32_t vertCount = 0;
    int32_t indCount = 0;

    static char distance[maxGridTextLength];
    static const float moveX = 2.0*GRID_WIDTH/WINDOW_WIDTH;
    static const float moveY = 2.0*GRID_HEIGHT/WINDOW_HEIGHT;
    float x = -1.0+moveX;
    float y =  1.0-moveY;

    if (DrawDistance) {
        for (int32_t j = 1; j < BOARD_Y-1; ++j) {
            for (int32_t i = 1; i < BOARD_X-1; ++i) {
                Node* n = &Board[j * BOARD_X + i];
                if (n->isMutable) {
                    snprintf(distance, maxGridTextLength, "%d", (int32_t)Board[getIndex(i, j)].distance);
                    int32_t distanceLen = strlen(distance);
                    makeTextData(distance, distanceLen, vec2(x, y), vec4(0.7, 0.7, 0.7, 1.0), 11, &grids[vertCount]);
                    vertCount += 4*distanceLen;
                    indCount += 2*distanceLen;
                }
                x += moveX;
            }
            x = -1.0+moveX;
            y -= moveY;
        }
        for (int32_t count = 0, num = 0; count < indCount; count += 2, num += 4) {
            gridsCount[count+0] = (Indice){num+0, num+1, num+2};
            gridsCount[count+1] = (Indice){num+2, num+3, num+0};
        }

        updateMeshBuffers(&GridMesh, vertCount, grids, indCount, gridsCount, GL_DYNAMIC_DRAW);
        const Camera gridCamera = getCamera(-1, 1, -1, 1, 0, -1);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        drawMesh(&GridMesh, &gridCamera);
        glBlendFunc (GL_ONE, GL_ZERO);
    }

    // Edges and Walls
    drawMesh(&WallsMesh, &WorldCamera);
    drawMesh(&EdgeMesh, &WorldCamera);

    // Particle Counter
    #define MaxParticleTextLenght 16
    static char particleText[MaxParticleTextLenght] = "00000 Particles";
    snprintf(particleText, MaxParticleTextLenght, "%5d Particles", ParticleCount);
    drawText(particleText, MaxParticleTextLenght-1, vec2(-1.0+16.0/WINDOW_WIDTH, -1.0 + (32.0 + 24.88)/WINDOW_HEIGHT), vec4(1.0, 1.0, 1.0, 1.0), 24.88);
}


static const float maxVelocity = 35.0;
static const float minVelocity = 15.0;
static const float friction = 0.98;
static const float particleMass = 0.2;
static const float bounciness = 0.9;
static void updateParticles(const int32_t BoardRootX, const int32_t BoardRootY) {
    static Vertice drawPoints[MAX_PARTICLES];

    for (int32_t i = 0; i < ParticleCount && Simulation; ++i) {
        int32_t indX = (int32_t)(Points[i].position.x)/GRID_WIDTH;
        int32_t indY = (int32_t)(WINDOW_HEIGHT-Points[i].position.y)/GRID_HEIGHT; // must be flipped to get the correct index

        // Removing particles inside a wall
        while (i < ParticleCount && !Board[getIndex(indX, indY)].isMutable) { // deleting points inside a wall
            --ParticleCount;
            Points[i] = Points[ParticleCount];
            indX = (int32_t)(Points[i].position.x)/GRID_WIDTH;
            indY = (int32_t)(WINDOW_HEIGHT-Points[i].position.y)/GRID_HEIGHT; // must be flipped to get the correct index
        }

        Points[i].velocity = addV(Points[i].velocity, Board[getIndex(indX, indY)].flow);

        Vector particleDirection = normalized(Points[i].velocity);
        float velForce = magnitude(Points[i].velocity);
        
        // Updating particle's velocity
        if (velForce > maxVelocity)
            Points[i].velocity = mulSV(particleDirection, maxVelocity);
        else if (indX == BoardRootX && indY == BoardRootY &&velForce < minVelocity) { // never slowing down in root position
            Points[i].velocity = mulSV(particleDirection, minVelocity);
        }
        Points[i].velocity = mulSV(Points[i].velocity, friction);

        // Randomness in velocity
        // cos(alpha) = dot(v, X-axis) / (len(v) * len(X-axis)) = v.x / len(v)
        float angle = acosf(particleDirection.x);
        if (Points[i].velocity.y < 0) // angle from 0 to 360
            angle = 2*M_PI - angle;
        angle += (M_PI/16.0) * rand()/RAND_MAX - (M_PI/32.0);
        Points[i].velocity = addV(Points[i].velocity, mulSV(vec2(cosf(angle), sinf(angle)), 0.25));
        
        // Calculating next position and checking wall hit
        Vector nextPos = addV(Points[i].position, mulSV(Points[i].velocity, particleMass));
        int32_t nextIndX = (int32_t)(nextPos.x)/GRID_WIDTH;
        int32_t nextIndY = (int32_t)(WINDOW_HEIGHT-nextPos.y)/GRID_HEIGHT; // must be flipped to get the correct index

        if (!Board[getIndex(nextIndX, nextIndY)].isMutable) { // Wall hit
            // Using DDA algorithm to calculate hit behavior
            Vector rayDirection = normalized(Points[i].velocity);
            Vector rayStepUnit = vec2(
                sqrtf(1+(rayDirection.y*rayDirection.y)/(rayDirection.x*rayDirection.x)),
                sqrtf(1+(rayDirection.x*rayDirection.x)/(rayDirection.y*rayDirection.y))
            );

            Vector rayPos = vec2( // origin of the starting point according to the board
                GRID_WIDTH * (float)indX,
                WINDOW_HEIGHT - GRID_HEIGHT * (float)indY
            );

            struct { int32_t x, y; } rayStep, rayGrid = {indX, indY};

            Vector rayLength;
            if (rayDirection.x < 0) {
                rayStep.x = -1;
                rayLength.x = (Points[i].position.x - rayPos.x) * rayStepUnit.x;
            }
            else {
                rayStep.x = 1;
                rayLength.x = (rayPos.x + GRID_WIDTH - Points[i].position.x) * rayStepUnit.x;
            }

            if (rayDirection.y > 0) {
                rayStep.y = -1;
                rayLength.y = (rayPos.y - Points[i].position.y) * rayStepUnit.y;
            }
            else {
                rayStep.y = 1;
                rayLength.y = (Points[i].position.y - rayPos.y + GRID_HEIGHT) * rayStepUnit.y;
            }

            enum {Verticly, Horizontally} colisoinDirection;
            float distance;
            while (Board[getIndex(rayGrid.x, rayGrid.y)].isMutable) {
                if (rayLength.x < rayLength.y) {
                    rayGrid.x += rayStep.x;
                    distance = rayLength.x;
                    rayLength.x += rayStepUnit.x*GRID_WIDTH;
                    colisoinDirection = Verticly;
                }
                else {
                    rayGrid.y += rayStep.y;
                    distance = rayLength.y;
                    rayLength.y += rayStepUnit.y*GRID_WIDTH;
                    colisoinDirection = Horizontally;
                }
            }

            // Hit behavior
            Vector colisionPoint = addV(Points[i].position, mulSV(rayDirection, distance));

            Vector rayDiffrence = subV(nextPos, colisionPoint);
            if (colisoinDirection == Verticly) {
                rayDiffrence.x = -rayDiffrence.x;
                Points[i].velocity.x = -Points[i].velocity.x * bounciness;
            }
            else {
                rayDiffrence.y = -rayDiffrence.y;
                Points[i].velocity.y = -Points[i].velocity.y * bounciness;
            }
            
            // Last check if the particle is not inside another wall after reflection
            Vector lastCheck = addV(colisionPoint, rayDiffrence);

            nextIndX = (int32_t)(lastCheck.x)/GRID_WIDTH;
            nextIndY = (int32_t)(WINDOW_HEIGHT-lastCheck.y)/GRID_HEIGHT;

            if (!Board[getIndex(nextIndX, nextIndY)].isMutable) {
                if (colisoinDirection == Verticly)
                    Points[i].velocity.y = -Points[i].velocity.y;
                else
                    Points[i].velocity.x = -Points[i].velocity.x;

                // Little padding to avoid sinking into the wall
                if (rayStep.x < 0)
                    colisionPoint.x += 1.0;
                else
                    colisionPoint.x -= 1.0;

                if (rayStep.y > 0)
                    colisionPoint.y += 1.0;
                else
                    colisionPoint.y -= 1.0;

                Points[i].position = colisionPoint;
            }
            else {
                Points[i].position = lastCheck;
            }
        }
        else { // Unless there is no wall
            Points[i].position = nextPos;
        }

        drawPoints[i] = (Vertice){Points[i].position, vec4(0.5, 0.2, 0.7, 1.0), 0.0, 0.0};
    }

    // Drawing all particles in single draw call
    updateMeshBuffers(&ParticleMesh, ParticleCount, drawPoints, 0, NULL, GL_DYNAMIC_DRAW);

    GLCall(glBindVertexArray(ParticleMesh.meshID));
    GLCall(glUseProgram(ParticleMesh.shader.shaderID));

    GLCall(glUniformMatrix4fv(glGetUniformLocation(ParticleMesh.shader.shaderID, "ortho"), 1, GL_FALSE, WorldCamera.m4Ortho.values));

    GLCall(glDrawArrays(GL_POINTS, 0, ParticleMesh.vertBuff.vertCount));

    GLCall(glUseProgram(0));
    GLCall(glBindVertexArray(0));
}

void updateBoard(GLFWwindow* window) {
    double xM, yM;
    glfwGetCursorPos(window, &xM, &yM);
    int32_t xInd = (int32_t)xM, yInd = (int32_t)yM;
    if (Simulation && !(xM < 0.0 || WINDOW_WIDTH <= xM || yM < 0 || WINDOW_HEIGHT <= yM)) {
        xInd /= GRID_WIDTH;
        yInd /= GRID_HEIGHT;
        flowField(xInd, yInd);
    }
    updateParticles(xInd, yInd);

    displayBoard();
}