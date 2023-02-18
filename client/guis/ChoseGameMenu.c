#include <sys/socket.h>
#include <pthread.h>
#include "ChoseGameMenu.h"
#include "../tictactoe/tictactoe.h"
#include "../connect4/connect4.h"
#include "MainMenu.h"
#include "../../events/EventManager.h"

SDL_bool * choseGameRunning = NULL;
int * choseGameClientSocket = NULL;
SDL_Renderer * choseGameRenderer = NULL;


void * choseGameNetworkListen();
void * choseGameSdlListen();

int choseGameMenu(SDL_Renderer * rendererMenu, int * socketClient){
    choseGameRunning = malloc(sizeof(SDL_bool));
    if(choseGameRunning==NULL){
        SDL_ExitWithError("ERROR ALLOCATING CHOSEGAMERUNNING SDLBOOL");
    }
    *choseGameRunning = SDL_TRUE;

    choseGameClientSocket = socketClient;
    choseGameRenderer = rendererMenu;

    pthread_t sdl_thread;
    pthread_create(&sdl_thread, NULL, choseGameSdlListen, NULL);

    pthread_t network_thread;
    pthread_create(&network_thread, NULL, (void*)choseGameNetworkListen, socketClient);


    while(*choseGameRunning){
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) {
            switch (event->type) {
                case SDL:
                    if(strcmp(event->instructions, "LEAVE") == 0){
                        send(*choseGameClientSocket, "LEAVEGAME", 9, 0);
                        *choseGameRunning = SDL_FALSE;
                        loadMainMenu();
                    } else if(strcmp(event->instructions, "TICTACTOE") == 0){
                        send(*choseGameClientSocket, "TICTACTOE", 9, 0);
                        *choseGameRunning = SDL_FALSE;
                        tictactoe(choseGameClientSocket);
                    } else if(strcmp(event->instructions, "CONNECT4") == 0){
                        send(*choseGameClientSocket, "NCONNECT4", 9, 0);
                        *choseGameRunning = SDL_FALSE;
                        connect4(choseGameClientSocket);
                    }
                    break;
                case NETWORK:
                    if(strcmp(event->instructions, "DISCONNECTED") == 0){
                        SDL_ExitWithError("DISCONNECTED FROM SERVER");
                    } else if(strcmp(event->instructions, "GAMEBREAK") == 0){
                        *choseGameRunning = SDL_FALSE;
                        loadMainMenu();
                    }
                    break;
                default:
                    break;
            }
        }
    }
}


void * choseGameSdlListen(){

    Button goBack;
    goBack.beginX = 20;
    goBack.beginY = 430;
    goBack.endX = 150;
    goBack.endY = 470;

    Button tictactoeButton;
    tictactoeButton.beginX = 5;
    tictactoeButton.beginY = 100;
    tictactoeButton.endX = 345;
    tictactoeButton.endY = 410;

    Button connect4Button;
    connect4Button.beginX = 375;
    connect4Button.beginY = 100;
    connect4Button.endX = 720;
    connect4Button.endY = 410;

    SDL_RenderClear(choseGameRenderer);

    createTextZoneCentered(choseGameRenderer, "Quel jeu choisir", 720/2 ,50,255, 255, 255,48);

    createButton(choseGameRenderer,tictactoeButton, "Morpion");
    createButton(choseGameRenderer,connect4Button, "Puissance 4");
    createButton(choseGameRenderer,goBack, "retour");

    updateRenderer(choseGameRenderer);

    while(*choseGameRunning){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    sendEvent(createEvent(SDL,"LEAVE"));
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>tictactoeButton.beginX && x<tictactoeButton.endX && y<tictactoeButton.endY && y>tictactoeButton.beginY){
                        sendEvent(createEvent(SDL,"TICTACTOE"));
                    }
                    if(x>connect4Button.beginX && x<connect4Button.endX && y<connect4Button.endY && y>connect4Button.beginY){
                        sendEvent(createEvent(SDL,"CONNECT4"));
                    }
                    if(x>goBack.beginX && x<goBack.endX && y<goBack.endY && y>goBack.beginY){
                        sendEvent(createEvent(SDL,"LEAVE"));
                    }
                    break;
                default:
                    break;
            }
        }
    }
    pthread_exit(NULL);
}


void * choseGameNetworkListen() {
    NG_Event *disconnectEvent = createEvent(NETWORK,"DISCONNECTED");

    while(*choseGameRunning){
        char data[12];
        memset(data, '\0', sizeof(data));
        if (recv(*choseGameClientSocket, data, sizeof(data), 0) <= 0) {
            sendEvent(disconnectEvent);
            break;
        }else {
            NG_Event *receivedDataEvent = malloc(sizeof(NG_Event));
            if(receivedDataEvent==NULL){
                SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT");
            }
            receivedDataEvent->type = NETWORK;
            unsigned long len = strlen(data);
            SDL_Log("[NETWORK_LISTENER] PACKET RECEIVED - LENGTH: %lu - CONTENT: \"%s\"", len,data);
            receivedDataEvent->instructions = malloc(sizeof(char)*len);
            if(receivedDataEvent->instructions==NULL){
                SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATAEVENT INSTRUCTIONS");
            }
            strcpy(receivedDataEvent->instructions,data);
            sendEvent(receivedDataEvent);
        }
    }
    pthread_exit(NULL);
}

