// #define GLEW_STATIC
#include <aplication.h>

int main() {
    if (initAplication()) return 1;

    mainLoop();

    destroyAplication();

    return 0;
}