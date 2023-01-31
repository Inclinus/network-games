#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "SDLUtils.h"

void changeColor(SDL_Renderer * renderer,int red, int green, int blue){
    if(SDL_SetRenderDrawColor(renderer,red,green,blue,SDL_ALPHA_OPAQUE) != 0)
        SDL_ExitWithError("Unable to change renderer color");
}

void createFilledRectangle(int x, int y, int w, int h, SDL_Renderer * renderer){
    SDL_Rect rectangle;
    rectangle.x = x;
    rectangle.y = y;
    rectangle.w = w;
    rectangle.h = h;
    if(SDL_RenderFillRect(renderer, &rectangle) != 0){
        SDL_ExitWithError("Unable to draw rectangle");
    }
}

void closeWindowAndRendererAndThrowError(SDL_Renderer * renderer, SDL_Window * window, const char *message){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_ExitWithError(message);
}

void updateRenderer(SDL_Renderer * renderer){
    SDL_RenderPresent(renderer);
}

void quitSDL(SDL_Renderer * renderer, SDL_Window * window){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void initSDL(){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        SDL_ExitWithError("SDL init failed");
    }
}

void SDL_ExitWithError(const char *message){
    SDL_Log("ERREUR : %s > %s\n", message, SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
}

void createTextZone(SDL_Renderer * renderer, const char * text, int posX, int posY, int width, int height, Uint8 red, Uint8 green, Uint8 blue){
    //this opens a font style and sets a size
    TTF_Font * font = TTF_OpenFont("../assets/Roboto-Regular.ttf", 24);

// this is the color in rgb format,
// maxing out all would give you the color white,
// and it will be your text's color
    SDL_Color white = {red, green, blue};

// as TTF_RenderText_Solid could only be used on
// SDL_Surface then you have to create the surface first
    SDL_Surface * surfaceMessage = TTF_RenderText_Solid(font,text, white);

// now you can convert it into a texture
    SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    SDL_Rect messageRectangle; //create a rect
    messageRectangle.x = posX;  //controls the rect's x coordinate
    messageRectangle.y = posY; // controls the rect's y coordinte
    messageRectangle.w = width; // controls the width of the rect
    messageRectangle.h = height; // controls the height of the rect

// (0,0) is on the top left of the window/screen,
// think a rect as the text's box,
// that way it would be very simple to understand

// Now since it's a texture, you have to put RenderCopy
// in your game loop area, the area where the whole code executes

// you put the renderer's name first, the Message,
// the crop size (you can ignore this if you don't want
// to dabble with cropping), and the rect which is the size
// and coordinate of your texture
    SDL_RenderCopy(renderer, messageTexture, NULL, &messageRectangle);
}
