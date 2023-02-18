#include "SettingsMenu.h"

SDL_bool settingsMenuRunning = SDL_FALSE;

void settingsMenu(SDL_Renderer * rendererMenu){
    settingsMenuRunning = SDL_TRUE;

    Button goBack;
    goBack.beginX = 20;
    goBack.beginY = 430;
    goBack.endX = 150;
    goBack.endY = 470;

    Button inputIp;
    inputIp.beginX = WIDTH/4;
    inputIp.beginY = HEIGHT/2-30;

    Button inputPort;
    inputPort.beginX = WIDTH/4;
    inputPort.beginY = HEIGHT/2+30;

    SDL_RenderClear(rendererMenu);

    createButton(rendererMenu,goBack, "retour");
    createButton(rendererMenu,inputIp, "ip du serveur :");
    createButton(rendererMenu,inputPort, "port du serveur :");

    while(settingsMenuRunning){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    settingsMenuRunning = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>goBack.beginX && x<goBack.endX && y<goBack.endY && y>goBack.beginY){
                        settingsMenuRunning = SDL_FALSE;
                        //MainMenu(rendererMenu);
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