#include "../../sdl-utils/SDLUtils.h"
#include <SDL2/SDL.h>

#define WIDTH 720
#define HEIGHT 480

typedef struct{
    int beginX;
    int beginY;
    int endX;
    int endY;
    int actionType;
} Button ;