#ifndef __INTERNAL_H__
#define __INTERNAL_H__


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "logging.h"

#define BOARD_X 20
#define BOARD_Y 20

#define GRID_WIDTH  40
#define GRID_HEIGHT 40


#define WINDOW_WIDTH  (BOARD_X*GRID_WIDTH)
#define WINDOW_HEIGHT (BOARD_Y*GRID_HEIGHT)

#define UV (WINDOW_WIDTH / WINDOW_HEIGHT)

#endif // __INTERNAL_H__