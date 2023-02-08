#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <regex.h>
#include "../../sdl-utils/SDLUtils.h"
#include "../../events/EventManager.h"


void displayBoard(int **board);
void placeSymbol(int ** board, int symbol, int px, int py);
void createSymbol(int symbol, int x, int y);
void yourTurn(SDL_bool * yourTurn,const int * socketClient);

SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;

SDL_bool program_launched = SDL_TRUE;
int * clientSocket;

int calculateLinePos(Sint32 x){
    if(x>405){
        return 2;
    } else if(x<195){
        return 0;
    } else {
        return 1;
    }
}

int calculateColumnPos(Sint32 y){
    if(y>405){
        return 2;
    } else if(y<195){
        return 0;
    } else {
        return 1;
    }
}

void *sdlListen() {
    while (program_launched) {
        SDL_Event event;
        //SDL_Log("BOUCLE DE JEU");

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_Log("SDL QUIT");
                    program_launched = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = calculateLinePos(event.button.x);
                    int y = calculateColumnPos(event.button.y);
                    SDL_Log("SDL BTN DOWN");
                    NG_Event *buttonDown = malloc(sizeof(NG_Event));
                    buttonDown->type = SDL;
                    buttonDown->instructions = malloc(sizeof(char)*11);
                    sprintf(buttonDown->instructions, "%d-%d", x, y);
                    sendEvent(buttonDown);
                    break;
                default:
                    break;
            }
        }
    }
}



void *networkListen() {
    NG_Event *disconnectEvent = malloc(sizeof(NG_Event));
    disconnectEvent->type = NETWORK;
    disconnectEvent->instructions = malloc(sizeof(char)*12);
    disconnectEvent->instructions = "DISCONNECTED";

    int player = 1;
    while (program_launched) {
        char data[9];
        memset(data, '\0', sizeof(data));
        if (recv(*clientSocket, data, 8, 0) <= 0) {
            sendEvent(disconnectEvent);
            break;
        } else {
            if (strcmp("WAITTURN", data) == 0) {
                NG_Event *enemyTurnEvent = malloc(sizeof(NG_Event));
                enemyTurnEvent->type = NETWORK;
                disconnectEvent->instructions = malloc(sizeof(char)*10);
                enemyTurnEvent->instructions = "ENEMYTURN";
                sendEvent(enemyTurnEvent);
                int px;
                int py;
                recv(*clientSocket, &px, sizeof(px), 0);
                recv(*clientSocket, &py, sizeof(py), 0);
                NG_Event *enemyPosEvent = malloc(sizeof(NG_Event)); // enemyPosEvent = Oxeaf & *enemyPosEvent = NG_EVENT{} &enemyPosEvent = 0xfk
                enemyPosEvent->type = NETWORK;
                enemyPosEvent->instructions = malloc(sizeof(char)*4);
                sprintf(enemyPosEvent->instructions, "%d-%d", px, py);
                sendEvent(enemyPosEvent);
            } else {
                NG_Event *receivedDataEvent = malloc(sizeof(NG_Event));
                receivedDataEvent->type = NETWORK;
                unsigned long len = strlen(data);
                SDL_Log("[NETWORK_LISTENER] PACKET RECEIVED - LENGTH: %lu - CONTENT: \"%s\"", len,data);
                receivedDataEvent->instructions = malloc(sizeof(char)*len);
                strcpy(receivedDataEvent->instructions,data);
                sendEvent(receivedDataEvent);
            }
        }
    }
}

SDL_bool tryPlace(SDL_bool isEnemy, int ** board, int px, int py){
    if(board[px][py]!=1 && board[px][py]!=2 && px > -1 && px < 3 && py > -1 && py < 3) {
        if(isEnemy){
            placeSymbol(board,2,px,py);
        } else {
            placeSymbol(board,1,px,py);
        }
        return SDL_TRUE;
    } else {
        return SDL_FALSE;
    }
}

int tictactoe(int * socketClient) {
    clientSocket = socketClient;
    initSDL();
    window = SDL_CreateWindow("MORPION", 50, 50, 600, 600, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);

    pthread_t network_listener;
    pthread_create(&network_listener, NULL, networkListen, NULL);
    pthread_t sdl_listener;
    pthread_create(&sdl_listener, NULL, sdlListen, NULL);

    int **board;
    int *row;
    board = malloc(sizeof(int *) * 3);

    for (int i = 0; i < 3; ++i) {
        row = malloc(sizeof(int) * 3);
        board[i] = row;

    }
    for (int i = 0; i < 3; ++i) {
        for (int y = 0; y < 3; ++y) {
            board[i][y] = 0;
        }
    }
    displayBoard(board);

    regex_t posRegex;
    if (regcomp(&posRegex, "^[0-9]-[0-9]$", REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Error: Could not compile regular expression\n");
        return 1;
    }

    SDL_bool yourTurn = SDL_FALSE;

    while (program_launched) {
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) {
            SDL_Log("TICTACTOE : EVENT RECEIVED");
            if (event->type == SDL) {
                SDL_Log("TICTACTOE : SDL EVENT RECEIVED %s",event->instructions);
                if (regexec(&posRegex, event->instructions, 0, NULL, 0) == 0) {
                    if(yourTurn==SDL_TRUE){
                        int x, y;
                        sscanf(event->instructions, "%d-%d", &x, &y);
                        if(tryPlace(SDL_FALSE,board,x,y)){
                            SDL_Log("Coup Possible ! \n");
                            send(*socketClient, &x, sizeof(x), 0);
                            send(*socketClient, &y, sizeof(y), 0);
                            displayBoard(board);
                            yourTurn = SDL_FALSE;
                        } else {
                            SDL_Log("Coup Impossible ! \n");
                        }
                    } else {
                        SDL_Log("Ce n'est pas votre tour ! \n");
                    }
                }
            } else if (event->type == NETWORK) {
                SDL_Log("TICTACTOE : NETWORK EVENT RECEIVED %s",event->instructions);
                if (strcmp(event->instructions, "DISCONNECTED") == 0) {
                    SDL_Log("DECONNECTE DU SERVEUR");
                    program_launched = SDL_FALSE;
                } else if (strcmp("YOURTURN", event->instructions) == 0) {
                    SDL_Log("C'est à vous de jouer ! \n");
                    yourTurn = SDL_TRUE;
                } else if (strcmp("ENEMYTURN", event->instructions) == 0) {
                    SDL_Log("C'est au tour de l'adversaire ! \n");
                    yourTurn = SDL_FALSE;
                } else if (regexec(&posRegex, event->instructions, 0, NULL, 0) == 0) {
                    int x, y;
                    sscanf(event->instructions, "%d-%d", &x, &y);
                    placeSymbol(board, 2, x, y);
                    displayBoard(board);
                } else if (strcmp("YOUWIN!!", event->instructions) == 0) {
                    SDL_Log("Vous avez gagné ! \n");
                    program_launched = SDL_FALSE;
                } else if (strcmp("YOULOSE!", event->instructions) == 0) {
                    SDL_Log("Vous avez perdu ! \n");
                    program_launched = SDL_FALSE;
                } else {
                    fprintf(stderr,"WTF IS THAT NETWORK EVENT : %s",event->instructions);
                }
            }
        }
    }
    quitSDL(renderer, window);
    close(*socketClient);
    return 0;
}

void displayBoard(int **board) {
    SDL_RenderClear(renderer);
    changeColor(renderer, 255, 255, 255);
    createFilledRectangle(0, 0, 600, 600, renderer);
    changeColor(renderer, 0, 0, 0);
    createFilledRectangle(0, 195, 600, 10, renderer);
    createFilledRectangle(0, 395, 600, 10, renderer);
    createFilledRectangle(195, 0, 10, 600, renderer);
    createFilledRectangle(395, 0, 10, 600, renderer);
    for (int i = 2; i >= 0; --i) {
        for (int y = 0; y < 3; ++y) {
            createSymbol(board[i][y],i,y);
        }
    }
    updateRenderer(renderer);
}

void placeSymbol(int **board, int symbol, int px, int py) {
    board[px][py] = symbol;
}

void createSymbol(int symbol, int x, int y){
    /*
     * 97-97    290-97    502-97
     * 97-290   290-290   502-290
     * 97-502   290-502   502-502
     */
    int centerX = 97 + x * (502 - 97) / 2;
    int centerY = 97 + y * (502 - 97) / 2;
    if(symbol==1){
        changeColor(renderer,0,0,255);
        createCircle(renderer,centerX,centerY,30);
    } else if(symbol==2){
        changeColor(renderer,255,0,0);
        SDL_RenderDrawLine(renderer,centerX-40,centerY-40,centerX+40,centerY+40);
    }
}


