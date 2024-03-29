#ifndef NETWORK_GAMES_SDLUtils_H
#define NETWORK_GAMES_SDLUtils_H


#include <SDL2/SDL.h>
typedef struct{
    int beginX;
    int beginY;
    int endX;
    int endY;
    int actionType;
} Button ;

// SDL Utils related to main window and renderer
void initSDL();
void quitSDL(SDL_Renderer * renderer, SDL_Window * window);
void updateRenderer(SDL_Renderer * renderer);

// SDL Utils to draw
void createFilledRectangle(int x, int y, int w, int h, SDL_Renderer * renderer);
void changeColor(SDL_Renderer * renderer,int red, int green, int blue);
SDL_Rect * createTextZoneCentered(SDL_Renderer * renderer, const char * text, int posX, int posY, Uint8 red, Uint8 green, Uint8 blue,int size);
SDL_Rect * createTextZone(SDL_Renderer * renderer, const char * text, int posX, int posY, Uint8 red, Uint8 green, Uint8 blue);
void createCircle(SDL_Renderer * renderer, int x, int y, int radius);
void createButton(SDL_Renderer * renderer, Button btn, const char * text);
void createButtonColor(SDL_Renderer * renderer, Button btn, const char * text, Uint8 red, Uint8 green, Uint8 blue);

// SDL Error utils
void SDL_ExitWithError(const char *message);
void closeWindowAndRendererAndThrowError(SDL_Renderer * renderer, SDL_Window * window, const char *message);

#endif