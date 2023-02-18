#include "ChoseGameMenu.h"


int choseGameMenu(SDL_Renderer * rendererMenu){

    SDL_bool choseGameRunning = SDL_TRUE;

    Button goBack;
    goBack.beginX = 20;
    goBack.beginY = 430;
    goBack.endX = 150;
    goBack.endY = 470;

    Button tictactoe;
    tictactoe.beginX = 5;
    tictactoe.beginY = 100;
    tictactoe.endX = 345;
    tictactoe.endY = 410;

    Button connect4;
    connect4.beginX = 375;
    connect4.beginY = 100;
    connect4.endX = 720;
    connect4.endY = 410;

    SDL_RenderClear(rendererMenu);

    createTextZoneCentered(rendererMenu, "Quel jeu choisir", 720/2 ,50,255, 255, 255,48);

    createButton(rendererMenu,tictactoe, "Morpion");
    createButton(rendererMenu,connect4, "Puissance 4");
    createButton(rendererMenu,goBack, "retour");

    updateRenderer(rendererMenu);

    while(choseGameRunning){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    choseGameRunning = SDL_FALSE;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>tictactoe.beginX && x<tictactoe.endX && y<tictactoe.endY && y>tictactoe.beginY){
                        return 1; // Launch Morpion
                    }
                    if(x>connect4.beginX && x<connect4.endX && y<connect4.endY && y>connect4.beginY){
                        return 2; //launch Puissance4
                    }
                    if(x>goBack.beginX && x<goBack.endX && y<goBack.endY && y>goBack.beginY){
                        return 3; //menu
                    }
                    else{
                        continue;
                    }
                    break;
                default:
                    break;
            }
        }
    }

}

