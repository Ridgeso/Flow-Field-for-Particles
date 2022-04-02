#include "particle.h"
#include "field/field.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

static Camera WorldCamera;
static int8_t Simulation = 1;
static enum {DrawDistance, DrawArrow, DrwaNothing} GridDraw = DrwaNothing;

extern Node Board[BOARD_X * BOARD_Y];

#define gridsAmount (BOARD_X*BOARD_Y - 2*(BOARD_X+BOARD_Y-2)) // number of grids without edges
#define maxGridTextLength 4
static Mesh DistanceMesh;
static Mesh ArrowMesh; static Vertice arrowVert[4*gridsAmount];

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
    
    {
        Indice distanceInd[2*gridsAmount * (maxGridTextLength-1)];
        for (int32_t count = 0, num = 0; count < 2*gridsAmount * (maxGridTextLength-1); count += 2, num += 4) {
            distanceInd[count+0] = (Indice){num+0, num+1, num+2};
            distanceInd[count+1] = (Indice){num+2, num+3, num+0};
        }
        initMesh(&DistanceMesh, DrawElements, GL_TRIANGLES, "../shaders/font.vert", "../shaders/font.frag", 0, NULL, 2*gridsAmount*(maxGridTextLength-1), distanceInd, GL_DYNAMIC_DRAW, 0, "../shaders/assets/font.png");
    }
    {
        for (int32_t i = 0; i < 4*gridsAmount; i += 4) {
            arrowVert[i+0] = (Vertice){vec(), vec4(0.3, 0.7, 1.0, 1.0), 0.3, 0.7};
            arrowVert[i+1] = (Vertice){vec(), vec4(0.3, 0.7, 1.0, 1.0), 0.3, 0.7};
            arrowVert[i+2] = (Vertice){vec(), vec4(0.3, 0.7, 1.0, 1.0), 0.3, 0.7};
            arrowVert[i+3] = (Vertice){vec(), vec4(0.3, 0.7, 1.0, 1.0), 0.3, 0.7};
        }

        Indice arrowInd[2*gridsAmount]; // little cheating, Indice store 3 indices so last element of count+0 and first of count+1 is one line
        for (int32_t count = 0, num = 0; count < 2*gridsAmount; count += 2, num += 4) {
            arrowInd[count+0] = (Indice){num+0, num+1, num+1}; // 1st | beginning of 2nd
            arrowInd[count+1] = (Indice){num+2, num+1, num+3}; // end of 2nd | 3rd
        }
        glLineWidth(1);
        initMesh(&ArrowMesh, DrawElements, GL_LINES, "../shaders/arrow.vert", "../shaders/f.frag", 4*gridsAmount, arrowVert, 2*gridsAmount, arrowInd, GL_DYNAMIC_DRAW, GL_TEXTURE31, NULL);
    }
    
    {
        Vertice edgesVert[] = {
            //                  POSITION                                        COLOR
            {vec2(0.0,                               0.0),              vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(WINDOW_WIDTH,                      0.0),              vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(WINDOW_WIDTH,            WINDOW_WIDTH ),              vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(0.0,                     WINDOW_HEIGHT),              vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},

            {vec2(GRID_WIDTH,              GRID_HEIGHT),                vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(WINDOW_WIDTH-GRID_WIDTH, GRID_HEIGHT),                vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(WINDOW_WIDTH-GRID_WIDTH, WINDOW_HEIGHT-GRID_HEIGHT),  vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7},
            {vec2(GRID_WIDTH,              WINDOW_HEIGHT-GRID_HEIGHT),  vec4(0.3, 0.7, 1.0, 1.0),    0.3, 0.7}
        };
        Indice edgesInd[] = {
            {0, 1, 5}, {0, 4, 5}, // top
            {2, 3, 6}, {3, 6, 7}, // bottom
            {0, 3, 7}, {0, 4, 7}, // left
            {1, 2, 5}, {2, 5, 6}  // right
        };
        initMesh(&EdgeMesh, DrawElements, GL_TRIANGLES, "../shaders/edge.vert", "../shaders/f.frag", 8, edgesVert, 8, edgesInd, GL_STATIC_DRAW, GL_TEXTURE31, NULL);
        initMesh(&WallsMesh, DrawElements, GL_TRIANGLES, "../shaders/edge.vert", "../shaders/f.frag", 0, NULL, 0, NULL, GL_DYNAMIC_DRAW, GL_TEXTURE31, NULL);
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
    initMesh(&ParticleMesh, DrawArray, GL_POINTS, "../shaders/particle.vert", "../shaders/f.frag", 4*BOARD_X*BOARD_Y, NULL, 2*BOARD_X*BOARD_Y, NULL, GL_DYNAMIC_DRAW, GL_TEXTURE31, NULL);
}

void particleKeysCallback(int32_t key, int32_t scancode, int32_t action, int32_t mod) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        if (Simulation) Simulation = 0;
        else Simulation = 1;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        GridDraw = DrawDistance;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        GridDraw = DrawArrow;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        GridDraw = DrwaNothing;
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
    deleteMesh(&DistanceMesh);
    deleteMesh(&ArrowMesh);
    deleteMesh(&WallsMesh);
    deleteMesh(&EdgeMesh);
    deleteMesh(&ParticleMesh);
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
    displayBoard();

    updateParticles(xInd, yInd);
}

static void displayBoard() {
    switch(GridDraw) {
    case DrawDistance:
        drawGridDistance(); break;
    case DrawArrow:
        drawGridArrow(); break;
    case DrwaNothing:
        break;
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

void drawGridDistance() {
    static Vertice gridsVert[4*gridsAmount * (maxGridTextLength-1)];

    int32_t vertCount = 0;
    int32_t indCount = 0;

    static char distance[maxGridTextLength];
    static const float moveX = 2.0*GRID_WIDTH/WINDOW_WIDTH;
    static const float moveY = 2.0*GRID_HEIGHT/WINDOW_HEIGHT;
    float x = -1.0+moveX;
    float y =  1.0-moveY;

    for (int32_t j = 1; j < BOARD_Y-1; ++j) {
        for (int32_t i = 1; i < BOARD_X-1; ++i) {
            Node* n = &Board[j * BOARD_X + i];
            if (n->isMutable) {
                snprintf(distance, maxGridTextLength, "%d", (int32_t)Board[getIndex(i, j)].distance);
                const int32_t distanceLen = strlen(distance);
                makeTextData(distance, distanceLen, vec2(x, y), vec4(0.7, 0.7, 0.7, 1.0), 11, &gridsVert[vertCount]);
                vertCount += 4*distanceLen;
                indCount += 2*distanceLen;
            }
            x += moveX;
        }
        x = -1.0+moveX;
        y -= moveY;
    }
    updateMeshBuffers(&DistanceMesh, vertCount, gridsVert, 0, NULL, GL_DYNAMIC_DRAW);
    DistanceMesh.indexBuff.indCount = indCount;

    const Camera gridCamera = getCamera(-1, 1, -1, 1, 0, -1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawMesh(&DistanceMesh, &gridCamera);
    glBlendFunc (GL_ONE, GL_ZERO);
}

void drawGridArrow() {
    static const Vector arrowPoints[] = {
        { 0,                 -GRID_HEIGHT/4.0, 0, 0},
        { 0,              3.0*GRID_HEIGHT/8.0, 0, 0},
        {-GRID_WIDTH/8.0,     GRID_HEIGHT/8.0, 0, 0},
        { GRID_WIDTH/8.0,     GRID_HEIGHT/8.0, 0, 0}
    };

    int32_t x = 1, y = 1;
    Vector arrowPosition = vec2(1.5*GRID_WIDTH, WINDOW_HEIGHT-1.5*GRID_HEIGHT);
    for (int32_t i = 0; i < gridsAmount; ++i) {
        Vector* arrowDirection = &Board[getIndex(x, y)].flow;

        // cos(alpha) = dot(v, Y-axis) / (len(v) * len(Y-axis)) = v.y / len(v) ; where len(v) = 1.0
        float angle = acosf(arrowDirection->y);
        if (arrowDirection->x > 0)
            angle = 2*M_PI - angle;
        
        arrowVert[i*4 + 0].pos = rotate(arrowPoints[0], angle);
        arrowVert[i*4 + 1].pos = rotate(arrowPoints[1], angle);
        arrowVert[i*4 + 2].pos = rotate(arrowPoints[2], angle);
        arrowVert[i*4 + 3].pos = rotate(arrowPoints[3], angle);

        // put arrows in world position
        arrowVert[i*4 + 0].pos = addV(arrowVert[i*4 + 0].pos, arrowPosition);
        arrowVert[i*4 + 1].pos = addV(arrowVert[i*4 + 1].pos, arrowPosition);
        arrowVert[i*4 + 2].pos = addV(arrowVert[i*4 + 2].pos, arrowPosition);
        arrowVert[i*4 + 3].pos = addV(arrowVert[i*4 + 3].pos, arrowPosition);

        arrowPosition.x += GRID_WIDTH;
        ++x;
        if (x == BOARD_X-1) {
            x = 1;
            ++y;
            arrowPosition.x = 1.5*GRID_WIDTH;
            arrowPosition.y -= GRID_HEIGHT;
        }
    }

    updateMeshBuffers(&ArrowMesh, 4*gridsAmount, arrowVert, 0, NULL, GL_DYNAMIC_DRAW);

    drawMesh(&ArrowMesh, &WorldCamera);
}



static const float maxVelocity = 35.0;
static const float minVelocity = 15.0;
static const float friction = 0.98;
static const float particleMass = 0.2;
static const float bounciness = 0.9;
static void updateParticles(const int32_t BoardRootX, const int32_t BoardRootY) {
    static Vertice pointsVert[MAX_PARTICLES];

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
        // cos(alpha) = dot(v, X-axis) / (len(v) * len(X-axis)) = v.x / len(v) ; where len(v) = 1.0
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

        pointsVert[i] = (Vertice){Points[i].position, (Vector){0.5, 0.2, 0.7, 1.0}, 0.0, 0.0};
    }

    // Drawing all particles in single draw call
    updateMeshBuffers(&ParticleMesh, ParticleCount, pointsVert, 0, NULL, GL_DYNAMIC_DRAW);

    drawMesh(&ParticleMesh, &WorldCamera);
}