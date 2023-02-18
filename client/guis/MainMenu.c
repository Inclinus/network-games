#include <pthread.h>
#include <SDL2/SDL_ttf.h>
#include <sys/socket.h>
#include "../../events/EventManager.h"
#include "ChoseGameMenu.h"
#include "SettingsMenu.h"
#include "CreditMenu.h"
#include "StatisticsMenu.h"
#include "../tictactoe/tictactoe.h"
#include "../connect4/connect4.h"

Button * choseGameButton;
Button * statisticsButton;
Button * creditButton;
Button * quitButton;
Button * optionButton;

void initButtons();
void * sdlClientListen();
void displayMenuMain();
void * networkMenuListen();

SDL_bool * mainMenuRunning = NULL;
SDL_bool * isClientRunning = NULL;

SDL_Renderer * rendererMenu = NULL;
SDL_Window * windowMenu = NULL;

int * mainMenuClientSocket = NULL;

SDL_bool * isInQueue = NULL;

void loadMainMenu(){
    clearQueues();
    mainMenuRunning = malloc(sizeof(SDL_bool));
    if(mainMenuRunning==NULL){
        SDL_ExitWithError("ERROR ALLOCATING MAINMENURUNNING SDLBOOL");
    }
    *mainMenuRunning = SDL_TRUE;

    isInQueue = malloc(sizeof(SDL_bool));
    if(isInQueue==NULL){
        SDL_ExitWithError("ERROR ALLOCATING ISINQUEUE SDLBOOL");
    }
    *isInQueue = SDL_FALSE;

    displayMenuMain();

    pthread_t sdl_thread;
    pthread_create(&sdl_thread, NULL, sdlClientListen, NULL);

    pthread_t network_thread;
    pthread_create(&network_thread, NULL, networkMenuListen, NULL);

    while(*mainMenuRunning){
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) {
            switch (event->type) {
                case SDL:
                    if(strcmp(event->instructions,"CHOSEGAME")==0){
                        *isInQueue = SDL_TRUE;
                        send(*mainMenuClientSocket, "QUEUE", 5, 0);
                        // TODO display text IN QUEUE
                    } else if(strcmp(event->instructions,"STATS")==0){
                        *mainMenuRunning = SDL_FALSE;
                        //statisticsMenu(rendererMenu);
                    } else if(strcmp(event->instructions,"CREDIT")==0){
                        *mainMenuRunning = SDL_FALSE;
                        creditMenu(rendererMenu);
                    } else if(strcmp(event->instructions,"SETTINGS")==0){
                        *mainMenuRunning = SDL_FALSE;
                        settingsMenu(rendererMenu);
                    }
                    break;
                case NETWORK:
                    if(strcmp(event->instructions,"LOBBYHOST")==0){
                        *isInQueue = SDL_FALSE;
                        *mainMenuRunning = SDL_FALSE;
                        pthread_cancel(network_thread);
                        pthread_cancel(sdl_thread);
                        choseGameMenu(rendererMenu,mainMenuClientSocket);
                    } else if(strcmp(event->instructions,"LOBBYJOUR")==0){
                        *isInQueue = SDL_FALSE;
                        // TODO display text "game found, waiting for choice"
                    } else if(strcmp(event->instructions,"TICTACTOE")==0){
                        *mainMenuRunning = SDL_FALSE;
                        pthread_cancel(network_thread);
                        pthread_cancel(sdl_thread);
                        tictactoe(mainMenuClientSocket);
                    } else if(strcmp(event->instructions,"NCONNECT4")==0){
                        *mainMenuRunning = SDL_FALSE;
                        pthread_cancel(network_thread);
                        pthread_cancel(sdl_thread);
                        connect4(mainMenuClientSocket);
                    } else if(strcmp(event->instructions,"GAMEBREAK")==0){
                        // TODO remove display text game found
                        //      add display text IN QUEUE
                    }
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
                    send(*mainMenuClientSocket,"LEAVEGAME",9,0);
                    *mainMenuRunning = SDL_FALSE;
                    *isClientRunning = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>choseGameButton->beginX && x<choseGameButton->endX && y<choseGameButton->endY && y>choseGameButton->beginY){
                        sendEvent(createEvent(SDL,"CHOSEGAME"));
                    } else if(x>statisticsButton->beginX && x<statisticsButton->endX && y<statisticsButton->endY && y>statisticsButton->beginY){
                        sendEvent(createEvent(SDL,"STATS"));
                    } else if(x>creditButton->beginX && x<creditButton->endX && y<creditButton->endY && y>creditButton->beginY){
                        sendEvent(createEvent(SDL,"CREDIT"));
                    } else if(x>quitButton->beginX && x<quitButton->endX && y<quitButton->endY && y>quitButton->beginY){
                        *mainMenuRunning = SDL_FALSE;
                        *isClientRunning = SDL_FALSE;
                    } else if(x>optionButton->beginX && x<optionButton->endX && y<optionButton->endY && y>optionButton->beginY){
                        sendEvent(createEvent(SDL,"SETTINGS"));
                    }
                    break;
                default:
                    break;
            }
        }
    }
    pthread_exit(NULL);
}


void * networkMenuListen() {
    NG_Event *disconnectEvent = createEvent(NETWORK,"DISCONNECTED");
    SDL_Log("LAUNCHING NETWORK THREAD MAINMENU");
    while(*mainMenuRunning){
        char data[10];
        memset(data, '\0', sizeof(data));
        if (recv(*mainMenuClientSocket, data, sizeof(data), 0) <= 0) {
            sendEvent(disconnectEvent);
            break;
        }else {
            NG_Event *receivedDataEvent = malloc(sizeof(NG_Event));
            if(receivedDataEvent==NULL){
                SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT");
            }
            receivedDataEvent->type = NETWORK;
            unsigned long len = strlen(data);
            SDL_Log("[MAINMENU NETWORK LISTENER] PACKET RECEIVED - LENGTH: %lu - CONTENT: \"%s\"", len,data);
            receivedDataEvent->instructions = malloc(sizeof(char)*len);
            if(receivedDataEvent->instructions==NULL){
                SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT INSTRUCTIONS");
            }
            strcpy(receivedDataEvent->instructions,data);
            sendEvent(receivedDataEvent);
        }
    }
    SDL_Log("EXITING NETWORK THREAD MAINMENU");
    pthread_exit(NULL);
}

void initButtons(){
    choseGameButton = malloc(sizeof(Button));
    if(choseGameButton==NULL){
        SDL_ExitWithError("ERROR WHILE ALLOCATING CHOSE GAME BUTTON");
    }
    choseGameButton->beginX = 180;
    choseGameButton->beginY = 150;
    choseGameButton->endX = 540;
    choseGameButton->endY = 200;

    statisticsButton = malloc(sizeof(Button));
    if(statisticsButton==NULL){
        SDL_ExitWithError("ERROR WHILE ALLOCATING CHOSE GAME BUTTON");
    }
    statisticsButton->beginX = 180;
    statisticsButton->beginY = 210;
    statisticsButton->endX = 540;
    statisticsButton->endY = 260;

    creditButton = malloc(sizeof(Button));
    if(creditButton==NULL){
        SDL_ExitWithError("ERROR WHILE ALLOCATING CREDIT BUTTON");
    }
    creditButton->beginX = 180;
    creditButton->beginY = 270;
    creditButton->endX = 540;
    creditButton->endY = 320;

    quitButton = malloc(sizeof(Button));
    if(quitButton==NULL){
        SDL_ExitWithError("ERROR WHILE ALLOCATING QUIT BUTTON");
    }
    quitButton->beginX = 180;
    quitButton->beginY = 330;
    quitButton->endX = 540;
    quitButton->endY = 380;

    optionButton = malloc(sizeof(Button));
    if(optionButton==NULL){
        SDL_ExitWithError("ERROR WHILE ALLOCATING SETTINGS BUTTON");
    }
    optionButton->beginX = 570;
    optionButton->beginY = 390;
    optionButton->endX = 700;
    optionButton->endY = 440;
}

void displayMenuMain(){
    SDL_RenderClear(rendererMenu);

    changeColor(rendererMenu,45,45,48);
    createFilledRectangle(0,0,WIDTH,HEIGHT,rendererMenu);

    createTextZoneCentered(rendererMenu, "Altino", WIDTH/2,50, 255, 255, 255,48);

    Button test;
    test.beginX = 0;
    test.beginY = 0;
    test.endX = 100;
    test.endY = 150;

    createButton(rendererMenu,test, "BOUTON DE TEST");

    createButton(rendererMenu,*choseGameButton, "Choisir une partie");
    createButton(rendererMenu,*statisticsButton,"Statistiques");
    createButton(rendererMenu,*creditButton,"Credit");
    createButton(rendererMenu,*quitButton,"Quitter");
    createButton(rendererMenu,*optionButton,"Options");

    updateRenderer(rendererMenu);
}

void initSDLGUIs(SDL_bool * clientRunning, int * clientSocket){
    mainMenuClientSocket = clientSocket;
    initSDL();
    TTF_Init();
    initButtons();

    windowMenu = SDL_CreateWindow("ALTINO - NETWORK GAMES",50,50,WIDTH,HEIGHT,0);
    rendererMenu = SDL_CreateRenderer(windowMenu,-1,0);

    isClientRunning = clientRunning;
}

