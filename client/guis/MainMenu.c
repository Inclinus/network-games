#include <pthread.h>
#include <SDL2/SDL_ttf.h>
#include "../../events/EventManager.h"
#include "ChoseGameMenu.h"
#include "SettingsMenu.h"
#include "CreditMenu.h"
#include "StatisticsMenu.h"

Button * choseGameButton;
Button * statisticsButton;
Button * creditButton;
Button * quitButton;
Button * optionButton;

void initButtons();
void createMenuMain();
void * sdlClientListen();
void * networkMenuListen(int *clientSocket);

SDL_bool * mainMenuRunning = NULL;
SDL_bool * isClientRunning = NULL;

SDL_Renderer * rendererMenu = NULL;
SDL_Window * windowMenu = NULL;

void loadMainMenu(SDL_bool * clientRunning, int * clientSocket){
    isClientRunning = clientRunning;
    *mainMenuRunning = SDL_TRUE;

    windowMenu = SDL_CreateWindow("ALTINO - NETWORK GAMES",50,50,WIDTH,HEIGHT,0);
    rendererMenu = SDL_CreateRenderer(windowMenu,-1,0);

    createMenuMain();

    pthread_t sdl_thread;
    pthread_create(&sdl_thread, NULL, sdlClientListen, NULL);

    pthread_t network_thread;
    pthread_create(&network_thread, NULL, (void*)networkMenuListen, clientSocket);

    while(*isClientRunning){
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) {
            switch (event->type) {
                case SDL:
                    if(strcmp(event->instructions,"QUEUE")==0){
                        // display text in queue
                        // add the guy to the queue
                    } else if(strcmp(event->instructions,"CHOSEGAME")==0){
                        *mainMenuRunning = SDL_FALSE;
                        choseGameMenu(rendererMenu);
                    } else if(strcmp(event->instructions,"STATS")==0){
                        *mainMenuRunning = SDL_FALSE;
                        statisticsMenu(rendererMenu);
                    } else if(strcmp(event->instructions,"CREDIT")==0){
                        *mainMenuRunning = SDL_FALSE;
                        creditMenu(rendererMenu);
                    } else if(strcmp(event->instructions,"SETTINGS")==0){
                        *mainMenuRunning = SDL_FALSE;
                        settingsMenu(rendererMenu);
                    }
                    break;
                case NETWORK:
                    break;
                default:
                    break;
            }

        }
    }
}

void * sdlClientListen(){
    while(*mainMenuRunning){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    *mainMenuRunning = SDL_FALSE;
                    *isClientRunning = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>choseGameButton->beginX && x<choseGameButton->endX && y<choseGameButton->endY && y<choseGameButton->beginY){
                        sendEvent(createEvent(SDL,"CHOSEGAME"));
                    } else if(x>statisticsButton->beginX && x<statisticsButton->endX && y<statisticsButton->endY && y<statisticsButton->beginY){
                        sendEvent(createEvent(SDL,"STATS"));
                    } else if(x>creditButton->beginX && x<creditButton->endX && y<creditButton->endY && y<creditButton->beginY){
                        sendEvent(createEvent(SDL,"CREDIT"));
                    } else if(x>quitButton->beginX && x<quitButton->endX && y<quitButton->endY && y<quitButton->beginY){
                        *mainMenuRunning = SDL_FALSE;
                        *isClientRunning = SDL_FALSE;
                    } else if(x>optionButton->beginX && x<optionButton->endX && y<optionButton->endY && y<optionButton->beginY){
                        sendEvent(createEvent(SDL,"SETTINGS"));
                    }
                    break;
                default:
                    break;
            }
        }
    }
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



void createMenuMain(){
    initSDL();
    TTF_Init();
    initButtons();

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

