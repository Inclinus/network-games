#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../sdl-utils/SDLUtils.h"
#include "sdl-client.h"

#define WIDTH 720
#define HEIGHT 480


SDL_bool program_launch = SDL_TRUE;

typedef struct{
    int beginX;
    int beginY;
    int endX;
    int endY;
    int actionType;
} Button ;

int MainMenu(SDL_Renderer * rendererMenu){

    initSDL();
    TTF_Init();

    changeColor(rendererMenu,45,45,48);
    createFilledRectangle(0,0,WIDTH,HEIGHT,rendererMenu);
    updateRenderer(rendererMenu);

    Button btn;
    btn.beginX = 180;
    btn.beginY = 150;
    btn.endX = 540;
    btn.endY = 200;

    SDL_RenderClear(rendererMenu);

    createTextZoneCentered(rendererMenu, "Altino", WIDTH/2,50, 255, 255, 255,48);

    for(int i=0;i<4;i++){

    changeColor(rendererMenu,0,122,204);
    createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,rendererMenu);

    changeColor(rendererMenu,45,45,48);
    createFilledRectangle(btn.beginX+5,btn.beginY+5,btn.endX-(btn.beginX+9),btn.endY-(btn.beginY+9),rendererMenu);

    btn.beginY=btn.beginY + 60;
    btn.endY=btn.endY + 60;


    }

    btn.beginY = 150;
    btn.endY = 200;

    for (int i = 0; i < 4; i++)
    {
    switch (i){
    case 0:
        createTextZoneCentered(rendererMenu, "Choisir une partie", btn.beginX+((btn.endX-btn.beginX)/2),btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
        break;
    case 1:
        createTextZoneCentered(rendererMenu, "Statistiques", btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
        break;
    case 2:
        createTextZoneCentered(rendererMenu, "Credit", btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
        break;
    case 3:
        createTextZoneCentered(rendererMenu, "Quitter",btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
        break;
    }

    btn.beginY=btn.beginY + 60;
    btn.endY=btn.endY + 60;
    }

    btn.beginX = 570;
    btn.endX = 700;

    changeColor(rendererMenu,0,122,204);
    createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,rendererMenu);

    changeColor(rendererMenu,45,45,48);
    createFilledRectangle(btn.beginX+5,btn.beginY+5,btn.endX-(btn.beginX+9),btn.endY-(btn.beginY+9),rendererMenu);
        

    createTextZoneCentered(rendererMenu, "Options", btn.beginX+((btn.endX-btn.beginX)/2),btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);

    
    updateRenderer(rendererMenu);

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
                        return 1; // Choisir le jeu 
                    }
                    if(x>btn.beginX && x<btn.endX && y<btn.endY+2*60 && y<btn.beginY+2*60){
                        return 2;// stat
                    }
                    if(x>btn.beginX && x<btn.endX && y<btn.endY+3*60 && y<btn.beginY+3*60){
                        Credit(rendererMenu);
                    }
                    if(x>btn.beginX && x<btn.endX && y<btn.endY+4*60 && y<btn.beginY+4*60){
                        return 3;
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

void Credit(SDL_Renderer * rendererMenu){

    Button option;
    option.beginX = 20;
    option.beginY = 430;
    option.endX = 150;
    option.endY = 470;

    SDL_RenderClear(rendererMenu);

    createTextZoneCentered(rendererMenu, "Credit : ",WIDTH/2,50, 255, 255, 255,48);

    changeColor(rendererMenu,0,122,204);
    createFilledRectangle(option.beginX,option.beginY,option.endX-option.beginX,option.endY-option.beginY,rendererMenu);

    changeColor(rendererMenu,45,45,48);
    createFilledRectangle(option.beginX+5,option.beginY+5,option.endX-(option.beginX+9),option.endY-(option.beginY+9),rendererMenu);

    createTextZoneCentered(rendererMenu, "retour", option.beginX+((option.endX-option.beginX)/2) ,option.beginY+((option.endY-option.beginY)/2), 255, 255, 255,24);

    updateRenderer(rendererMenu);

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
                        MainMenu(rendererMenu);
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

int Game(SDL_Renderer * rendererMenu){
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

    SDL_RenderClear(rendererMenu);

    createTextZoneCentered(rendererMenu, "Quel jeu choisir", 720/2 ,50,255, 255, 255,48);

    for(int i=0;i<2;i++){

    changeColor(rendererMenu,0,122,204);
    createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,rendererMenu);

    changeColor(rendererMenu,45,45,48);
    createFilledRectangle(btn.beginX+5,btn.beginY+5,btn.endX-(btn.beginX+9),btn.endY-(btn.beginY+9),rendererMenu);
    
    if (i==0)
    {
        createTextZoneCentered(rendererMenu, "Morpion", btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2),255, 255, 255,24);
    }else{
        createTextZoneCentered(rendererMenu, "Puissance 4", btn.beginX+((btn.endX-btn.beginX)/2) ,btn.beginY+((btn.endY-btn.beginY)/2), 255, 255, 255,24);
    }

    
    btn.beginX=btn.beginX + 370;
    btn.endX=btn.endX + 370;
    }

        changeColor(rendererMenu,0,122,204);
    createFilledRectangle(option.beginX,option.beginY,option.endX-option.beginX,option.endY-option.beginY,rendererMenu);

    changeColor(rendererMenu,45,45,48);
    createFilledRectangle(option.beginX+5,option.beginY+5,option.endX-(option.beginX+9),option.endY-(option.beginY+9),rendererMenu);
    
    createTextZoneCentered(rendererMenu, "retour", option.beginX+((option.endX-option.beginX)/2) ,option.beginY+((option.endY-option.beginY)/2), 255, 255, 255,24);



    updateRenderer(rendererMenu);

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
                    if(x>5 && x<345 && y<410 && y>100){
                        return 1; // Launch Morpion
                    }
                    if(x>365 && x<715 && y<410 && y>100){
                        return 2; //launch Puissance4
                    }
                    if(x>20 && x<150 && y<470 && y>430){
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

// MainMenu() -> Choix de jeu -> 1
//            -> Statististiqies -> 2
//            -> OSEF      
//            -> Quit -> 3

void Options(SDL_Renderer * rendererMenu){
    Button option;
    option.beginX = 20;
    option.beginY = 430;
    option.endX = 150;
    option.endY = 470;

    Button input;
    input.beginX = WIDTH/4;
    input.beginY = HEIGHT/2-30;

    createTextZoneCentered(rendererMenu, "ip du serveur :", input.beginX ,input.beginY, 255, 255, 255,24);
    createTextZoneCentered(rendererMenu, "port du serveur :", input.beginX,input.beginY+1*60, 255, 255, 255,24);

    SDL_RenderClear(rendererMenu);

    changeColor(rendererMenu,0,122,204);
    createFilledRectangle(option.beginX,option.beginY,option.endX-option.beginX,option.endY-option.beginY,rendererMenu);

    changeColor(rendererMenu,45,45,48);
    createFilledRectangle(option.beginX+5,option.beginY+5,option.endX-(option.beginX+9),option.endY-(option.beginY+9),rendererMenu);
    
    createTextZoneCentered(rendererMenu, "retour", option.beginX+((option.endX-option.beginX)/2) ,option.beginY+((option.endY-option.beginY)/2), 255, 255, 255,24);    


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
                    if(x>20 && x<150 && y<470 && y>430){
                        MainMenu(rendererMenu);
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

int authentLoginOrRegister(SDL_Renderer * rendererMain){

    Button btn;
    btn.beginX = 5;
    btn.beginY = 100;
    btn.endX = 345;
    btn.endY = 410;

    createTextZoneCentered(rendererMain, "Altino", WIDTH/2,50, 255, 255, 255,48);
    
        
    
}