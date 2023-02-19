#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "SDLUtils.h"
#include <math.h>

void changeColor(SDL_Renderer * renderer,int red, int green, int blue){
    if(SDL_SetRenderDrawColor(renderer,red,green,blue,SDL_ALPHA_OPAQUE) != 0)// Définition de la couleur, renvoie une erreur si la fonction ne retourne pas 0
        SDL_ExitWithError("Unable to change renderer color");
}

void createFilledRectangle(int x, int y, int w, int h, SDL_Renderer * renderer){
    SDL_Rect rectangle; // Définition des paramètre du rectangle
    rectangle.x = x; // Définie les coordonnées de X
    rectangle.y = y; // Définie les coordonnées de Y
    rectangle.w = w; // Définie la longueur
    rectangle.h = h; // Définie la largeur
    if(SDL_RenderFillRect(renderer, &rectangle) != 0){ // Dessine le rectangle, renvoie une erreur si la fonction ne retourne pas 0
        SDL_ExitWithError("Unable to draw rectangle");
    }
}

void closeWindowAndRendererAndThrowError(SDL_Renderer * renderer, SDL_Window * window, const char *message){ // Permet de fermer une fenêtre SDL proprement
    SDL_DestroyRenderer(renderer); // Détruit le rendu de la fenêtre
    SDL_DestroyWindow(window); // Detruit la fenêtre SDL
    SDL_ExitWithError(message); // Ecrit que la fenêtre est détruite
}

void updateRenderer(SDL_Renderer * renderer){ //Met à jour le rendu SDL
    SDL_RenderPresent(renderer);
}

void quitSDL(SDL_Renderer * renderer, SDL_Window * window){ // Permet de quitter entièrement le rendu SDL
    SDL_DestroyRenderer(renderer); // Détruit le rendu de la fenêtre
    SDL_DestroyWindow(window); // Detruit la fenêtre SDL
    SDL_Quit(); // Quitte entièrement SDL
}

void initSDL(){ // Demarre les lib SDL et SDL_ttf, renvoie une erreur si les fonctions ne retournent pas 0
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        SDL_ExitWithError("SDL init failed");
    }
    if(TTF_Init()==-1){
        SDL_ExitWithError("SDL_ttf init failed");
    }
}

void SDL_ExitWithError(const char *message){ // Permet de faire des logs en cas d'erreur SDL
    SDL_Log("ERREUR : %s > %s\n", message, SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
}

SDL_Rect * createTextZone(SDL_Renderer * renderer, const char * text, int posX, int posY, Uint8 red, Uint8 green, Uint8 blue){  // Permet de faire un text en SDL en utilisant utilisant des coordonnées centrer par rapport autext

    TTF_Font * font = TTF_OpenFont("assets/Roboto-Regular.ttf", 24); // Choisis la police d'écriture
    if(font==NULL){// Renvoie une erreur si la police n'est pas correct
        SDL_ExitWithError("FONT NULL");
    }

    SDL_Color color = {red, green, blue}; // Choisis la couleur

    SDL_Surface * surfaceMessage; // Définie la surface prise par le message
    if((surfaceMessage = TTF_RenderText_Solid(font,text, color))==NULL){ // Erreur si le message n'occupe pas de place
        SDL_ExitWithError("SURFACE MESSAGE NULL");
    }

    SDL_Texture* messageTexture = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //Ajoute la texture au message
    if(messageTexture==NULL){ // Erreur si la texture est null
        SDL_ExitWithError("TEXTURE MESSAGE NULL");
    }

    SDL_Rect * messageRectangle = malloc(sizeof(SDL_Rect)); //Crée un rectangle
    if(messageRectangle==NULL){
        SDL_ExitWithError("MESSAGE RECTANGLE NULL");
    }
    messageRectangle->x = posX; // Choisis les coordonnées de X
    messageRectangle->y = posY; // Choisis les coordonnées de X
    messageRectangle->w = surfaceMessage->w; // Choisis la longeur prise par le message
    messageRectangle->h = surfaceMessage->h; // Choisis la largeur prise par le message

    SDL_RenderCopy(renderer, messageTexture, NULL, messageRectangle);// Ecrit le message sur le rendu SDL
    return messageRectangle;
}

void createCircle(SDL_Renderer * renderer, int x, int y, int radius) // Permet de créer un cercle dans le rendu SDL
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

SDL_Rect * createTextZoneCentered(SDL_Renderer * renderer, const char * text,int posX, int posY, Uint8 red, Uint8 green, Uint8 blue, int size){

    TTF_Font * font = TTF_OpenFont("assets/Roboto-Regular.ttf", size);
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
    messageRectangle->x = posX-(surfaceMessage->w/2);  //controls the rect's x coordinate
    messageRectangle->y = posY-(surfaceMessage->h/2); // controls the rect's y coordinte
    messageRectangle->w = surfaceMessage->w; // controls the width of the rect
    messageRectangle->h = surfaceMessage->h; // controls the height of the rect

    SDL_RenderCopy(renderer, messageTexture, NULL, messageRectangle);
    return messageRectangle;
}

void createButton(SDL_Renderer * renderer, Button btn,const char * text){
    SDL_Log("CREATING BUTTON");
    int posX = btn.beginX+((btn.endX-btn.beginX)/2);
    int posY = btn.beginY+((btn.endY-btn.beginY)/2);

    changeColor(renderer,0,122,204);
    createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,renderer);

    changeColor(renderer,45,45,48);
    createFilledRectangle(btn.beginX+5,btn.beginY+5,btn.endX-(btn.beginX+9),btn.endY-(btn.beginY+9),renderer);


    createTextZoneCentered(renderer, text,posX,posY, 255, 255, 255,24);
}

void createButtonColor(SDL_Renderer * renderer, Button btn,const char * text, Uint8 red, Uint8 green, Uint8 blue){

    int posX = btn.beginX+((btn.endX-btn.beginX)/2);
    int posY = btn.beginY+((btn.endY-btn.beginY)/2);

    changeColor(renderer,red,green,blue);
    createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,renderer);

    changeColor(renderer,45,45,48);
    createFilledRectangle(btn.beginX+5,btn.beginY+5,btn.endX-(btn.beginX+9),btn.endY-(btn.beginY+9),renderer);


    createTextZoneCentered(renderer, text,posX,posY, 255, 255, 255,24);
}