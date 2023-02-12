#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "SDLUtils.c"

#define MAX_WIDTH 720;
#define MAX_HEIGHT 480;

SDL_Renderer * renderer = NULL;
SDL_Window * window = NULL;

SDL_bool program_launch = SDL_TRUE;

typedef struct{
    int beginX;
    int beginY;
    int endX;
    int endY;
    int actionType;
} Button ;

int main(){

    initSDL();
    TTF_Init();
    window = SDL_CreateWindow("MORPION",50,50,720,480,0);
    renderer = SDL_CreateRenderer(window,-1,0);


    changeColor(renderer,45,45,48);
    createFilledRectangle(0,0,720,480,renderer);
    updateRenderer(renderer);

    Game();

}

void MainMenu(){

    Button btn;
    btn.beginX = 180;
    btn.beginY = 150;
    btn.endX = 540;
    btn.endY = 200;

    SDL_RenderClear(renderer);

    createTextZoneCentered(renderer, "Altino", 720/2,50, 255, 255, 255,48);

    for(int i=0;i<4;i++){

    changeColor(renderer,0,122,204);
    createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,renderer);

    changeColor(renderer,45,45,48);
    createFilledRectangle(btn.beginX+5,btn.beginY+5,btn.endX-(btn.beginX+9),btn.endY-(btn.beginY+9),renderer);

    btn.beginY=btn.beginY + 60;
    btn.endY=btn.endY + 60;


    }

    btn.beginY = 150;
    btn.endY = 200;

    for (int i = 0; i < 4; i++)
    {
    switch (i){
    case 0:
        createTextZoneCentered(renderer, "Choisir une partie", btn.beginX+((btn.endX-btn.beginX)/2),btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
        break;
    case 1:
        createTextZoneCentered(renderer, "Statistiques", btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
        break;
    case 2:
        createTextZoneCentered(renderer, "Credit", btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
        break;
    case 3:
        createTextZoneCentered(renderer, "Quitter",btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
        break;
    }

    btn.beginY=btn.beginY + 60;
    btn.endY=btn.endY + 60;
    }

    btn.beginX = 570;
    btn.endX = 700;

    changeColor(renderer,0,122,204);
    createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,renderer);

    changeColor(renderer,45,45,48);
    createFilledRectangle(btn.beginX+5,btn.beginY+5,btn.endX-(btn.beginX+9),btn.endY-(btn.beginY+9),renderer);
        

    createTextZoneCentered(renderer, "Options", btn.beginX+((btn.endX-btn.beginX)/2),btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);

    
    updateRenderer(renderer);

    btn.beginX = 180;
    btn.beginY = 150;
    btn.endX = 540;
    btn.endY = 200;



    while(program_launch){
        SDL_Event event;    
        int swap = 0;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    program_launch = SDL_FALSE;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>btn.beginX && x<btn.endX && y<btn.endY+60 && y<btn.beginY+60){
                        Game();
                    }
                    if(x>btn.beginX && x<btn.endX && y<btn.endY+2*60 && y<btn.beginY+2*60){
                        Credit();
                    }
                    if(x>btn.beginX && x<btn.endX && y<btn.endY+3*60 && y<btn.beginY+3*60){
                        Credit();
                    }
                    if(x>btn.beginX && x<btn.endX && y<btn.endY+4*60 && y<btn.beginY+4*60){
                        program_launch = SDL_FALSE;
                        break;
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

void Credit(){

    Button option;
    option.beginX = 20;
    option.beginY = 430;
    option.endX = 150;
    option.endY = 470;

    SDL_RenderClear(renderer);

    createTextZoneCentered(renderer, "Credit : ",720/2,50, 255, 255, 255,48);

    changeColor(renderer,0,122,204);
    createFilledRectangle(option.beginX,option.beginY,option.endX-option.beginX,option.endY-option.beginY,renderer);

    changeColor(renderer,45,45,48);
    createFilledRectangle(option.beginX+5,option.beginY+5,option.endX-(option.beginX+9),option.endY-(option.beginY+9),renderer);

    createTextZoneCentered(renderer, "retour", option.beginX+((option.endX-option.beginX)/2) ,option.beginY+((option.endY-option.beginY)/2), 255, 255, 255,24);

    updateRenderer(renderer);

    while(program_launch){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    program_launch = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>option.beginX && x<option.endX && y<option.endY && y>option.beginY){
                        MainMenu();
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

void Game(){
    Button option;
    option.beginX = 20;
    option.beginY = 430;
    option.endX = 150;
    option.endY = 470;

    Button btn;
    btn.beginX = 5;
    btn.beginY = 100;
    btn.endX = 345;
    btn.endY = 410;

    SDL_RenderClear(renderer);

    createTextZoneCentered(renderer, "Quel jeu choisir", 720/2 ,50,255, 255, 255,48);

    for(int i=0;i<2;i++){

    changeColor(renderer,0,122,204);
    createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,renderer);

    changeColor(renderer,45,45,48);
    createFilledRectangle(btn.beginX+5,btn.beginY+5,btn.endX-(btn.beginX+9),btn.endY-(btn.beginY+9),renderer);
    
    if (i==0)
    {
        createTextZoneCentered(renderer, "Morpion", btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2),255, 255, 255,24);
    }else{
        createTextZoneCentered(renderer, "Puissance 4", btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
    }

    
    btn.beginX=btn.beginX + 370;
    btn.endX=btn.endX + 370;
    }

        changeColor(renderer,0,122,204);
    createFilledRectangle(option.beginX,option.beginY,option.endX-option.beginX,option.endY-option.beginY,renderer);

    changeColor(renderer,45,45,48);
    createFilledRectangle(option.beginX+5,option.beginY+5,option.endX-(option.beginX+9),option.endY-(option.beginY+9),renderer);
    
    createTextZoneCentered(renderer, "retour", option.beginX+((option.endX-option.beginX)/2) ,option.beginY+((option.endY-option.beginY)/2), 255, 255, 255,24);



    updateRenderer(renderer);

    while(program_launch){
        SDL_Event event;    
        int swap = 0;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    program_launch = SDL_FALSE;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>5 && x<345 && y<410 && y>100){
                        MainMenu();
                    }
                    if(x>365 && x<715 && y<410 && y>100){
                        Credit();
                    }
                    if(x>20 && x<150 && y<470 && y>430){
                        MainMenu();
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

void Options(){

}