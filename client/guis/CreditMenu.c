#include "CreditMenu.h"
#include "MainMenu.h"

SDL_bool creditMenuRunning = SDL_FALSE;

void creditMenu(SDL_Renderer * rendererMenu){
    creditMenuRunning = SDL_TRUE;

    Button option;
    option.beginX = 20;
    option.beginY = 430;
    option.endX = 150;
    option.endY = 470;

    SDL_RenderClear(rendererMenu);

    createTextZoneCentered(rendererMenu, "Credit : ",WIDTH/2,50, 255, 255, 255,48);

    createButton(rendererMenu,option, "retour");

    updateRenderer(rendererMenu);

    while(creditMenuRunning){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    creditMenuRunning = SDL_TRUE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>option.beginX && x<option.endX && y<option.endY && y>option.beginY){
                        creditMenuRunning = SDL_FALSE;
                        loadMainMenu();
                    }
                    else{
                        continue;
                    }
                default:
                    break;
            }
        }
    }
}