#include "StatisticsMenu.h"
#include "MainMenu.h"

    SDL_bool * statisticsMenuRunning = NULL;

void statisticsMenu(SDL_Renderer * rendererMenu){

    statisticsMenuRunning = malloc(sizeof(SDL_bool));
    if(statisticsMenuRunning==NULL){
        SDL_ExitWithError("ERROR ALLOCATING CREDITMENURUNNING SDLBOOL");
    }
    *statisticsMenuRunning = SDL_TRUE;

    Button option;
    option.beginX = 20;
    option.beginY = 430;
    option.endX = 150;
    option.endY = 470;

    SDL_RenderClear(rendererMenu);

    createTextZoneCentered(rendererMenu, "Statistics : ",720/2,50, 255, 255, 255,48);


    createTextZoneCentered(rendererMenu,"Connect 4 Win : ",720/2,120, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,"Connect 4 Lose : ",720/2,160, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,"Connect 4 Draw : ",720/2,200, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,"TicTacToe Win : ",720/2,240, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,"TicTacToe Lose : ",720/2,280, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,"TicTacToe Draw : ",720/2,320, 255,255,255, 24);

    createButton(rendererMenu,option, "retour");

    updateRenderer(rendererMenu);

    while(*statisticsMenuRunning){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    *statisticsMenuRunning = SDL_TRUE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>option.beginX && x<option.endX && y<option.endY && y>option.beginY){
                        *statisticsMenuRunning = SDL_FALSE;
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
