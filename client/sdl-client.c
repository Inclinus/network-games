#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "../sdl-utils/SDLUtils.h"
#include "sdl-client.h"
#include "../events/EventManager.h"
#include <pthread.h>
#include <arpa/inet.h>

SDL_bool program_launch = SDL_TRUE;

Button * choseGameButton;
Button * statisticsButton;
Button * creditButton;
Button * quitButton;
Button * optionButton;

void initButtons(){
    choseGameButton->beginX = 180;
    choseGameButton->beginY = 150;
    choseGameButton->endX = 540;
    choseGameButton->endY = 200;

    statisticsButton->beginX = 180;
    statisticsButton->beginY = 210;
    statisticsButton->endX = 540;
    statisticsButton->endY = 260;

    creditButton->beginX = 180;
    creditButton->beginY = 270;
    creditButton->endX = 540;
    creditButton->endY = 320;

    quitButton->beginX = 180;
    quitButton->beginY = 330;
    quitButton->endX = 540;
    quitButton->endY = 380;

    optionButton->beginX = 570;
    optionButton->beginY = 390;
    optionButton->endX = 700;
    optionButton->endY = 440;
}


void createMenuMain(SDL_Renderer * rendererMenu){
    initSDL();
    TTF_Init();

    SDL_RenderClear(rendererMenu);

    changeColor(rendererMenu,45,45,48);
    createFilledRectangle(0,0,WIDTH,HEIGHT,rendererMenu);

    createTextZoneCentered(rendererMenu, "Altino", WIDTH/2,50, 255, 255, 255,48);

    createButton(rendererMenu,*choseGameButton, "Choisir une partie");
    createButton(rendererMenu,*statisticsButton,"Statistiques");
    createButton(rendererMenu,*creditButton,"Credit");
    createButton(rendererMenu,*quitButton,"Quitter");
    createButton(rendererMenu,*optionButton,"Options");

    updateRenderer(rendererMenu);
}

void * sdlClientListen(){
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
                    if(x>choseGameButton->beginX && x<choseGameButton->endX && y<choseGameButton->endY && y<choseGameButton->beginY){
                        sendEvent(createEvent(SDL,"CHOSEGAME"));
                        //Game(rendererMenu);
                    } else if(x>statisticsButton->beginX && x<statisticsButton->endX && y<statisticsButton->endY && y<statisticsButton->beginY){
                        sendEvent(createEvent(SDL,"STATS"));
                    } else if(x>creditButton->beginX && x<creditButton->endX && y<creditButton->endY && y<creditButton->beginY){
                        sendEvent(createEvent(SDL,"CREDIT"));
                        //Credit(rendererMenu);
                    } else if(x>quitButton->beginX && x<quitButton->endX && y<quitButton->endY && y<quitButton->beginY){
                        program_launch = SDL_FALSE;
                    } else if(x>optionButton->beginX && x<optionButton->endX && y<optionButton->endY && y<optionButton->beginY){
                        sendEvent(createEvent(SDL,"OPTIONS"));
                        //Options(rendererMenu);
                    }
                    break;
                default:
                    break;
            }
        }
    }

}

void creditMenu(SDL_Renderer * rendererMenu){

    Button option;
    option.beginX = 20;
    option.beginY = 430;
    option.endX = 150;
    option.endY = 470;

    SDL_RenderClear(rendererMenu);

    createTextZoneCentered(rendererMenu, "Credit : ",WIDTH/2,50, 255, 255, 255,48);

    createButton(rendererMenu,option, "retour");

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

int choseGameMenu(SDL_Renderer * rendererMenu){
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


void optionsMenu(SDL_Renderer * rendererMenu){
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
                    if(x>goBack.beginX && x<goBack.endX && y<goBack.endY && y>goBack.beginY){
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

int authentLoginOrRegister(SDL_Renderer * rendererMain){

    Button btn;
    btn.beginX = 5;
    btn.beginY = 100;
    btn.endX = 345;
    btn.endY = 410;

    createTextZoneCentered(rendererMain, "Altino", WIDTH/2,50, 255, 255, 255,48);
}

void loadMainMenu(SDL_bool * program_launched, int * clientSocket){

    SDL_bool mainMenu_launched = SDL_TRUE;

    SDL_Renderer * rendererMenu = NULL;
    SDL_Window * windowMenu = NULL;

    windowMenu = SDL_CreateWindow("ALTNO - NETWORK GAMES",50,50,720,480,0);
    rendererMenu = SDL_CreateRenderer(windowMenu,-1,0);

    createMenuMain(rendererMenu);

    pthread_t sdl_thread;
    pthread_create(&sdl_thread, NULL, sdlClientListen, NULL);

    pthread_t network_thread;
    pthread_create(&network_thread, NULL, (void*)networkMenuListen, clientSocket);

    while(program_launched){
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) {
            switch (event->type) {
                case SDL:
                    break;
                case NETWORK:
                    break;
                default:
                    break;
            }

        }
    }


    pthread_exit(&sdl_thread);
    pthread_exit(&network_thread);
}


void * networkMenuListen(int *clientSocket) {
    NG_Event *disconnectEvent = malloc(sizeof(NG_Event));
    if(disconnectEvent==NULL){
        SDL_ExitWithError("ERROR ALLOCATING DISCONNECTEVENT");
    }
    disconnectEvent->type = NETWORK;
    disconnectEvent->instructions = malloc(sizeof(char)*12);
    if(disconnectEvent->instructions==NULL){
        SDL_ExitWithError("ERROR ALLOCATING DISCONNECTEVENT INSTRUCTIONS");
    }
    disconnectEvent->instructions = "DISCONNECTED";

//        char data[9];
//        memset(data, '\0', sizeof(data));
//        if (recv(*clientSocket, data, 8, 0) <= 0) {
//            sendEvent(disconnectEvent);
//            break;
//        } else {
//            NG_Event *receivedDataEvent = malloc(sizeof(NG_Event));
//            if(receivedDataEvent==NULL){
//                SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT");
//            }
//            receivedDataEvent->type = NETWORK;
//            unsigned long len = strlen(data);
//            SDL_Log("[NETWORK_LISTENER] PACKET RECEIVED - LENGTH: %lu - CONTENT: \"%s\"", len,data);
//            receivedDataEvent->instructions = malloc(sizeof(char)*len);
//            if(receivedDataEvent->instructions==NULL){
//                SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT INSTRUCTIONS");
//            }
//            strcpy(receivedDataEvent->instructions,data);
//            sendEvent(receivedDataEvent);
//        }
}
