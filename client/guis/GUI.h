#include "../../sdl-utils/SDLUtils.h"
#include <SDL2/SDL.h>


typedef struct{
    int beginX;
    int beginY;
    int endX;
    int endY;
    int actionType;
} Button ;


#define WIDTH 720
#define HEIGHT 480