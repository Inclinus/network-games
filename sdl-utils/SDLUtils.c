#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "SDLUtils.h"
#include <math.h>

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
    if(TTF_Init()==-1){
        SDL_ExitWithError("SDL_ttf init failed");
    }
}

void SDL_ExitWithError(const char *message){
    SDL_Log("ERREUR : %s > %s\n", message, SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
}

SDL_Rect * createTextZone(SDL_Renderer * renderer, const char * text, int posX, int posY, Uint8 red, Uint8 green, Uint8 blue){

    TTF_Font * font = TTF_OpenFont("assets/Roboto-Regular.ttf", 24);
    if(font==NULL){
        SDL_ExitWithError("FONT NULL");
    }

    SDL_Color color = {red, green, blue};

    SDL_Surface * surfaceMessage;
    if((surfaceMessage = TTF_RenderText_Solid(font,text, color))==NULL){
        SDL_ExitWithError("SURFACE MESSAGE NULL");
    }

    SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    if(messageTexture==NULL){
        SDL_ExitWithError("TEXTURE MESSAGE NULL");
    }

    SDL_Rect * messageRectangle = malloc(sizeof(SDL_Rect)); //create a rect
    if(messageRectangle==NULL){
        SDL_ExitWithError("MESSAGE RECTANGLE NULL");
    }
    messageRectangle->x = posX;  //controls the rect's x coordinate
    messageRectangle->y = posY; // controls the rect's y coordinte
    messageRectangle->w = surfaceMessage->w; // controls the width of the rect
    messageRectangle->h = surfaceMessage->h; // controls the height of the rect

    SDL_RenderCopy(renderer, messageTexture, NULL, messageRectangle);
    return messageRectangle;
}

void createCircle(SDL_Renderer * renderer, int x, int y, int radius)
{
    int x1, y1;
    float angle;

    for (angle = 0; angle < 360; angle += 0.1)
    {
        x1 = x + (radius * cos(angle * M_PI / 180));
        y1 = y + (radius * sin(angle * M_PI / 180));
        SDL_RenderDrawPoint(renderer, x1, y1);
    }
}
