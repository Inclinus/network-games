#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <regex.h>
#include "../../sdl-utils/SDLUtils.h"
#include "../../events/EventManager.h"

void displayBoard(int **board);

SDL_bool tryPlace(SDL_bool isEnemy, int ** board, int px, int py);
void placeSymbol(int ** board, int symbol, int px, int py);
void createSymbol(int symbol, int x, int y);

int calculateLinePos(Sint32 x);
int calculateColumnPos(Sint32 y);

void * sdlListen();
void *networkListen();

void setDisplayInfo(char * displayInfo);
void setDisplayFeedback(char * displayFeedback);

SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;

SDL_bool program_launched = SDL_TRUE;

SDL_bool quitForcedByPlayer = SDL_FALSE;

int * clientSocket;

char * tictactoeDisplayInfo;
char * tictactoeDisplayFeedback;

int tictactoe(int * socketClient) {
    clientSocket = socketClient;
    initSDL();
    window = SDL_CreateWindow("MORPION", 50, 50, 600, 700, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);

    pthread_t network_listener;
    pthread_create(&network_listener, NULL, networkListen, NULL);
    pthread_t sdl_listener;
    pthread_create(&sdl_listener, NULL, sdlListen, NULL);

    int **board;
    int *row;
    board = malloc(sizeof(int *) * 3);
    if(board==NULL){
        SDL_ExitWithError("ERROR ALLOCATING BOARD");
    }

    for (int i = 0; i < 3; ++i) {
        row = malloc(sizeof(int) * 3);
        if(row==NULL){
            SDL_ExitWithError("ERROR ALLOCATING ROW");
        }
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
                            yourTurn = SDL_FALSE;
                        } else {
                            SDL_Log("Coup Impossible ! \n");
                            setDisplayFeedback("Coup Impossible !");
                        }
                    } else {
                        SDL_Log("Ce n'est pas votre tour ! \n");
                        setDisplayFeedback("Ce n'est pas votre tour !");
                    }
                }
            } else if (event->type == NETWORK) {
                SDL_Log("TICTACTOE : NETWORK EVENT RECEIVED %s",event->instructions);
                if (strcmp(event->instructions, "DISCONNECTED") == 0) {
                    SDL_Log("DECONNECTE DU SERVEUR");
                    setDisplayInfo("DECONNECTE DU SERVEUR");
                    program_launched = SDL_FALSE;
                } else if (strcmp("YOURTURN", event->instructions) == 0) {
                    SDL_Log("C'est à vous de jouer ! \n");
                    setDisplayInfo("C'est à vous de jouer !");
                    yourTurn = SDL_TRUE;
                } else if (strcmp("ENEMYTURN", event->instructions) == 0) {
                    SDL_Log("C'est au tour de l'adversaire ! \n");
                    setDisplayInfo("C'est au tour de l'adversaire !");
                    yourTurn = SDL_FALSE;
                } else if (regexec(&posRegex, event->instructions, 0, NULL, 0) == 0) {
                    int x, y;
                    sscanf(event->instructions, "%d-%d", &x, &y);
                    placeSymbol(board, 2, x, y);
                } else if (strcmp("YOUWIN!!", event->instructions) == 0) {
                    SDL_Log("Vous avez gagné ! \n");
                    setDisplayInfo("Vous avez gagné !");
                    program_launched = SDL_FALSE;
                } else if (strcmp("YOULOSE!", event->instructions) == 0) {
                    SDL_Log("Vous avez perdu ! \n");
                    setDisplayInfo("Vous avez perdu !");
                    program_launched = SDL_FALSE;
                }  else if (strcmp("DRAWDRAW", event->instructions) == 0) {
                    SDL_Log("Personne n'a gagné ! \n");
                    setDisplayInfo("Personne n'a gagné !");
                    program_launched = SDL_FALSE;
                }
                else {
                    fprintf(stderr,"WTF IS THAT NETWORK EVENT : %s",event->instructions);
                }
            }
            displayBoard(board);
            setDisplayFeedback(" ");
        }
    }
    if(!quitForcedByPlayer){
        sleep(5);
    }
    quitSDL(renderer, window);
    close(*socketClient);
    return 0;
}

void *sdlListen() {
    while (program_launched) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_Log("SDL QUIT");
                    quitForcedByPlayer = SDL_TRUE;
                    program_launched = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    SDL_Log("SDL BTN DOWN");
                    int x = calculateLinePos(event.button.x);
                    int y = calculateColumnPos(event.button.y);
                    if(y!=-1) {
                        NG_Event *buttonDown = malloc(sizeof(NG_Event));
                        if (buttonDown == NULL) {
                            SDL_ExitWithError("ERROR ALLOCATING BUTTONDOWNEVENT");
                        }
                        buttonDown->type = SDL;
                        buttonDown->instructions = malloc(sizeof(char) * 11);
                        if (buttonDown->instructions == NULL) {
                            SDL_ExitWithError("ERROR ALLOCATING BUTTONDOWNEVENT INSTRUCTIONS");
                        }
                        sprintf(buttonDown->instructions, "%d-%d", x, y);
                        sendEvent(buttonDown);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}


void *networkListen() {
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

    while (program_launched) {
        char data[9];
        memset(data, '\0', sizeof(data));
        if (recv(*clientSocket, data, 8, 0) <= 0) {
            sendEvent(disconnectEvent);
            break;
        } else {
            if (strcmp("WAITTURN", data) == 0) {
                sendEvent(createEvent(NETWORK,"ENEMYTURN"));
                int px;
                int py;
                recv(*clientSocket, &px, sizeof(px), 0);
                recv(*clientSocket, &py, sizeof(py), 0);
                NG_Event *enemyPosEvent = malloc(sizeof(NG_Event)); // enemyPosEvent = Oxeaf & *enemyPosEvent = NG_EVENT{} &enemyPosEvent = 0xfk
                if(enemyPosEvent==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYPOSEVENT");
                }
                enemyPosEvent->type = NETWORK;
                enemyPosEvent->instructions = malloc(sizeof(char)*4);
                if(enemyPosEvent->instructions==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYPOSEVENT INSTRUCTIONS");
                }
                sprintf(enemyPosEvent->instructions, "%d-%d", px, py);
                sendEvent(enemyPosEvent);
            } else {
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
    }
}

void setDisplayInfo(char * displayInfo){
    tictactoeDisplayInfo = displayInfo;
}
void setDisplayFeedback(char * displayFeedback){
    tictactoeDisplayFeedback = displayFeedback;
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

void displayBoard(int **board) {
    SDL_RenderClear(renderer);
    changeColor(renderer, 255, 255, 255);
    createFilledRectangle(0, 0, 600, 700, renderer);
    changeColor(renderer, 0, 0, 0);
    createFilledRectangle(0, 295, 600, 10, renderer);
    createFilledRectangle(0, 495, 600, 10, renderer);
    createFilledRectangle(195, 100, 10, 600, renderer);
    createFilledRectangle(395, 100, 10, 600, renderer);

    if(tictactoeDisplayInfo!=NULL){
        createTextZone(renderer,tictactoeDisplayInfo,200,10,0,150,0);
    }
    if(tictactoeDisplayFeedback!=NULL){
        createTextZone(renderer,tictactoeDisplayFeedback,200,50,150,0,0);
    }

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
    int centerY = 197 + y * (502 - 97) / 2;
    if(symbol==1){
        changeColor(renderer,0,0,255);
        createCircle(renderer,centerX,centerY,30);
    } else if(symbol==2){
        changeColor(renderer,255,0,0);
        SDL_RenderDrawLine(renderer,centerX-40,centerY-40,centerX+40,centerY+40);
    }
}

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
    if(y>505){
        return 2;
    } else if(y<295 && y>100){
        return 0;
    } else if(y>295){
        return 1;
    } else {
        return -1;
    }
}


