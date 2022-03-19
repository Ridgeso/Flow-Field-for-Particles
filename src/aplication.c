#include "aplication.h"
#include <math.h>

static GLFWwindow* window;

int32_t initAplication() {
    ASSERT(!glfwInit(), COULD NOT INITIALIZE GLFW);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle", NULL, NULL);
    ASSERT(!window, COULD NOT CREATE THE WINDOW);

    glfwMakeContextCurrent(window);

    ASSERT(glewInit() != GLEW_OK, COULD NOT INITIALIZE GLEW);

    
    initBoard();
    initRenderer();

    glfwSetKeyCallback(window, keysCallBack);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSwapInterval(1);
    
    return 0;
}

void keysCallBack(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mod) {
    // printf("[KEY] %d [CODE] %d [ACTION] %d [MOD] %d\n", key, scancode, action, mod);
    particleKeysCallback(key, scancode, action, mod);
}

void mouse_button_callback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) {
    double xM, yM;
    glfwGetCursorPos(window, &xM, &yM);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        addRemoveWalls(xM, yM);
}

void mainLoop() {

    glEnable(GL_BLEND);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.07f, 0.13f, 0.17f, 0.5f);

        // Whole algorithm
        updateBoard(window);
    
#ifdef DEBUG
        updateFrameCounter();
#else
        drawFrameCounter();
#endif // DEBUG
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

#ifdef DEBUG
void updateFrameCounter() {
    static Deltatime FPS;
    FPS.currTime = glfwGetTime();
    FPS.timeDiff = FPS.currTime - FPS.prevTime;
    ++FPS.frameCounter;
    if (FPS.timeDiff >= 1.0 / 20.0) {
        const int32_t maxTitle = 15 + 6 + 6 + 8 + 1;
        char newTitle[maxTitle];
        snprintf(newTitle, maxTitle, "Particle | FPS %6.3f | ms %8.5f", (1.0 / FPS.timeDiff) * FPS.frameCounter, (FPS.timeDiff / FPS.frameCounter) * 1000);
        glfwSetWindowTitle(window, newTitle);
        FPS.prevTime = FPS.currTime;
        FPS.frameCounter = 0;
    }
}
#else
#define PRECISION 12
void drawFrameCounter() {
    static Deltatime FPS;
    static char frames[PRECISION] = "0000.00 FPS";
    FPS.currTime = glfwGetTime();
    FPS.timeDiff = FPS.currTime - FPS.prevTime;
    ++FPS.frameCounter;
    if (FPS.timeDiff >= 1.0 / 8.0) {
        snprintf(frames, PRECISION, "%7.2f FPS", (1.0 / FPS.timeDiff) * FPS.frameCounter);
        FPS.prevTime = FPS.currTime;
        FPS.frameCounter = 0;
    }
    drawText(frames, PRECISION-1, vec2(-1.0+16.0/WINDOW_WIDTH, 0.98), vec4(1.0, 1.0, 1.0, 1.0), 24.88);
}
#endif // DEBUG

void destroyAplication() {
    deleteRenderer();

    destroyBoard();

    glfwTerminate();
}