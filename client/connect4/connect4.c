#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <sys/socket.h>
#include <pthread.h>
#include <regex.h>
#include <unistd.h>
#include "../../sdl-utils/SDLUtils.h"
#include "connect4.h"
#include "../../events/EventManager.h"

void checkFull(char **game, int *win);
void checkLine(char **game, char color, int line, int *win);
void checkColumn(char **game, char color, int column, int *win);
void checkDiagonals(char **game, char color, int line, int column, int *win);
void checkWin(char **game, char color, int column, int line, int *win);

SDL_bool tryPlay(char **game, int column, char color);

void initializeGame(char **game);
void displayGameBoard(char **game);

void *sdlListener();
void *networkListener();

void debugConnectBoard(char **game);
int calculateBoardColumnPos(Sint32 x);

void setDisplayedInfo(char * text);
void setDisplayedFeedback(char * text);


int SIZE = 7;

SDL_Window *windowConnect4 = NULL;
SDL_Renderer *rendererConnect4 = NULL;

int WINDOW_WIDTH = 700;
int WINDOW_HEIGHT = 800;

char RED = 'R';
char BLUE = 'B';

// SDL Bool to do the game loop
SDL_bool connect_launched = SDL_TRUE;

SDL_bool connect4QuitForcedByPlayer = SDL_FALSE;

char * actualDisplayedInfo;
char * actualDisplayedFeedback;

int * connect4ClientSocket;

int connect4(int * socketClient) {
    clearQueues();
    setDisplayedInfo("Bienvenue !");
    connect4ClientSocket = socketClient;
    initSDL();
    windowConnect4 = SDL_CreateWindow("CONNECT 4", 50, 50, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    rendererConnect4 = SDL_CreateRenderer(windowConnect4, -1, 0);

    pthread_t network_listener;
    pthread_create(&network_listener, NULL, networkListener, NULL);
    pthread_t sdl_listener;
    pthread_create(&sdl_listener, NULL, sdlListener, NULL);

    char **game = malloc(sizeof(char *) * SIZE);
    if(game==NULL){
        SDL_ExitWithError("ERROR ALLOCATING CHAR ** GAME");
    }

    initializeGame(game);
    SDL_Log("Voici le plateau de départ :\n");
    displayGameBoard(game);

    regex_t posRegex;
    if (regcomp(&posRegex, "^POS[0-9]$", REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Error: Could not compile regular expression\n");
        SDL_Log("Error: Could not compile regular expression\n");
        return 1;
    }
    regex_t enemyPosRegex;
    if (regcomp(&enemyPosRegex, "^ENEMY[0-9]$", REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Error: Could not compile regular expression\n");
        SDL_Log("Error: Could not compile regular expression\n");
        return 1;
    }

    SDL_bool * yourTurn = malloc(sizeof(SDL_bool));
    if(yourTurn==NULL){
        SDL_ExitWithError("ERROR ALLOCATING YOURTURN BOOL");
    }
    *yourTurn = SDL_FALSE;

    while (connect_launched) {
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) {
            SDL_Log("CONNECT4 : EVENT RECEIVED");
            if (event->type == SDL) {
                SDL_Log("CONNECT4 : SDL EVENT RECEIVED %s",event->instructions);
                if (regexec(&posRegex, event->instructions, 0, NULL, 0) == 0) {
                    if(*yourTurn){
                        int column;
                        sscanf(event->instructions, "POS%d", &column);
                        if(tryPlay(game,column,BLUE)){
                            debugConnectBoard(game);
                            SDL_Log("Coup Possible ! \n");
                            send(*connect4ClientSocket, &column, sizeof(column), 0);
                            *yourTurn = SDL_FALSE;
                        } else {
                            SDL_Log("Coup Impossible ! \n");
                        }
                    } else {
                        SDL_Log("Ce n'est pas votre tour ! \n");
                        setDisplayedFeedback("Ce n'est pas votre tour !");
                    }
                } else {
                    SDL_Log("SDL EVENT RECEIVED %s DOES NOT MATCH REGEX",event->instructions);
                }
            } else if (event->type == NETWORK) {
                SDL_Log("CONNECT4 : NETWORK EVENT RECEIVED %s",event->instructions);
                if (strcmp(event->instructions, "DISCONNECTED") == 0) {
                    SDL_Log("DECONNECTE DU SERVEUR");
                    setDisplayedInfo("DECONNECTE DU SERVEUR");
                    connect_launched = SDL_FALSE;
                } else if (strcmp("YOURTURN", event->instructions) == 0) {
                    SDL_Log("C'est à vous de jouer ! \n");
                    setDisplayedInfo("C'est à vous de jouer !");
                    *yourTurn = SDL_TRUE;
                } else if (strcmp("ENEMYTURN", event->instructions) == 0) {
                    SDL_Log("C'est au tour de l'adversaire ! \n");
                    setDisplayedInfo("C'est au tour de l'adversaire !");
                    *yourTurn = SDL_FALSE;
                } else if (regexec(&enemyPosRegex, event->instructions, 0, NULL, 0) == 0) {
                    int y;
                    sscanf(event->instructions, "ENEMY%d", &y);
                    tryPlay(game,y,RED);
                    debugConnectBoard(game);
                    displayGameBoard(game);
                } else if (strcmp("YOUWIN!!", event->instructions) == 0) {
                    SDL_Log("Vous avez gagné ! \n");
                    setDisplayedInfo("Vous avez gagné !");
                    connect_launched = SDL_FALSE;
                    pthread_cancel(network_listener);
                    pthread_cancel(sdl_listener);
                } else if (strcmp("YOULOSE!", event->instructions) == 0) {
                    SDL_Log("Vous avez perdu ! \n");
                    setDisplayedInfo("Vous avez perdu !");
                    connect_launched = SDL_FALSE;
                    pthread_cancel(network_listener);
                    pthread_cancel(sdl_listener);
                } else if (strcmp("EQUALITY", event->instructions) == 0) {
                    SDL_Log("Personne n'a gagné ! \n");
                    setDisplayedInfo("Personne n'a gagné !");
                    connect_launched = SDL_FALSE;
                    pthread_cancel(network_listener);
                    pthread_cancel(sdl_listener);
                } else {
                    fprintf(stderr,"WTF IS THAT NETWORK EVENT : %s",event->instructions);
                }
            }
            displayGameBoard(game);
            setDisplayedFeedback(" ");
        }

    }
    if(!connect4QuitForcedByPlayer){
        sleep(10);
    }
    quitSDL(rendererConnect4, windowConnect4);
    close(*socketClient);
    return 0;
}

void *sdlListener() {
    while (connect_launched) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_Log("SDL QUIT");
                    connect4QuitForcedByPlayer = SDL_TRUE;
                    connect_launched = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int column = calculateBoardColumnPos(event.button.x);
                    SDL_Log("SDL BTN DOWN");
                    NG_Event *buttonDown = malloc(sizeof(NG_Event));
                    if(buttonDown==NULL){
                        SDL_ExitWithError("ERROR ALLOCATING BUTTONDOWN EVENT");
                    }
                    buttonDown->type = SDL;
                    buttonDown->instructions = malloc(sizeof(char)*11);
                    if(buttonDown->instructions==NULL){
                        SDL_ExitWithError("ERROR ALLOCATING BUTTONDOWN INSTRUCTIONS");
                    }
                    sprintf(buttonDown->instructions, "POS%d", column);
                    sendEvent(buttonDown);
                    break;
                default:
                    break;
            }
        }
    }
}



void *networkListener() {
    NG_Event *disconnectEvent = malloc(sizeof(NG_Event));
    if(disconnectEvent==NULL){
        SDL_ExitWithError("ERROR ALLOCATING DISCONNECTEVENT EVENT");
    }
    disconnectEvent->type = NETWORK;
    disconnectEvent->instructions = malloc(sizeof(char)*12);
    if(disconnectEvent->instructions==NULL){
        SDL_ExitWithError("ERROR ALLOCATING DISCONNECTEVENT INSTRUCTIONS");
    }
    disconnectEvent->instructions = "DISCONNECTED";

    while (connect_launched) {
        char data[9];
        memset(data, '\0', sizeof(data));
        if (recv(*connect4ClientSocket, data, 8, 0) <= 0) {
            sendEvent(disconnectEvent);
            break;
        } else {
            if (strcmp("PING", data) == 0){
                send(*connect4ClientSocket, "PONG", 4, 0);
            } else if (strcmp("WAITTURN", data) == 0) {
                NG_Event *enemyTurnEvent = malloc(sizeof(NG_Event));
                if(enemyTurnEvent==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYTURN EVENT");
                }
                enemyTurnEvent->type = NETWORK;
                enemyTurnEvent->instructions = malloc(sizeof(char)*10);
                if(enemyTurnEvent->instructions==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYTURN INSTRUCTIONS");
                }
                enemyTurnEvent->instructions = "ENEMYTURN";
                sendEvent(enemyTurnEvent);
                int enemyPos;
                recv(*connect4ClientSocket, &enemyPos, sizeof(enemyPos), 0);
                NG_Event *enemyPosEvent = malloc(sizeof(NG_Event));
                if(enemyPosEvent==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYPOS EVENT");
                }
                enemyPosEvent->type = NETWORK;
                enemyPosEvent->instructions = malloc(sizeof(char)*4);
                if(enemyPosEvent->instructions==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING ENEMYPOS INSTRUCTIONS");
                }
                sprintf(enemyPosEvent->instructions, "ENEMY%d", enemyPos);
                sendEvent(enemyPosEvent);
            } else {
                NG_Event *receivedDataEvent = malloc(sizeof(NG_Event));
                if(receivedDataEvent==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATA EVENT");
                }
                receivedDataEvent->type = NETWORK;
                unsigned long len = strlen(data);
                SDL_Log("[NETWORK_LISTENER] PACKET RECEIVED - LENGTH: %lu - CONTENT: \"%s\"", len,data);
                receivedDataEvent->instructions = malloc(sizeof(char)*len);
                if(receivedDataEvent->instructions==NULL){
                    SDL_ExitWithError("ERROR ALLOCATING RECEIVEDDATA INSTRUCTIONS");
                }
                strcpy(receivedDataEvent->instructions,data);
                sendEvent(receivedDataEvent);
            }
        }
    }
}


void setDisplayedInfo(char * text){
    actualDisplayedInfo = text;
}

void setDisplayedFeedback(char * text){
    actualDisplayedFeedback = text;
}


SDL_bool tryPlay(char **game, int column, char color) {
    if (column < 1 || column > 7) return SDL_FALSE;
    int count = SIZE - 1;
    while (game[column - 1][count] != '+') {
        count--;
    }
    if (count < 0) return SDL_FALSE;
    game[column - 1][count] = color;
    SDL_Log("[DEBUG] You placed your token on column %d, he fell to the line %d.\n", column, count + 1);
    return SDL_TRUE;
}

void debugConnectBoard(char **game) {
    SDL_Log("\n-----------------------------\n");
    for (int i = 0; i < SIZE; i++) {
        SDL_Log("| %d ", i + 1);
    }
    SDL_Log("|\n");
    SDL_Log("-----------------------------\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            SDL_Log("| %c ", game[j][i]);
        }
        SDL_Log("|\n");
    }
    SDL_Log("-----------------------------\n");
}

int calculateBoardColumnPos(Sint32 x){
    if(x>600){
        return 7;
    } else if(x>500){
        return 6;
    } else if(x>400){
        return 5;
    } else if(x>300){
        return 4;
    } else if(x>200){
        return 3;
    } else if(x>100){
        return 2;
    } else {
        return 1;
    }
}

void createSymbolCircle(char symbol, int x, int y){
    int centerX = 50 + x * 100;
    int centerY = 150 + y * 100;
    if(symbol==BLUE){
        changeColor(rendererConnect4,0,0,255);
        createCircle(rendererConnect4,centerX,centerY,30);
    } else if(symbol==RED){
        changeColor(rendererConnect4,255,0,0);
        createCircle(rendererConnect4,centerX,centerY,30);
    }
}

void displayGameBoard(char **game) {
    SDL_RenderClear(rendererConnect4);
    changeColor(rendererConnect4, 255, 255, 255);
    createFilledRectangle(0, 0, 700, 800, rendererConnect4);
    changeColor(rendererConnect4, 0, 0, 0);
    createFilledRectangle(0, 100, 700, 5, rendererConnect4);
    createFilledRectangle(0, 795, 700, 5, rendererConnect4);
    createFilledRectangle(0, 100, 5, 700, rendererConnect4);
    createFilledRectangle(695, 100, 5, 700, rendererConnect4);

    createFilledRectangle(0, 195, 700, 10, rendererConnect4);
    createFilledRectangle(0, 295, 700, 10, rendererConnect4);
    createFilledRectangle(0, 395, 700, 10, rendererConnect4);
    createFilledRectangle(0, 495, 700, 10, rendererConnect4);
    createFilledRectangle(0, 595, 700, 10, rendererConnect4);
    createFilledRectangle(0, 695, 700, 10, rendererConnect4);

    createFilledRectangle(95, 100, 10, 700, rendererConnect4);
    createFilledRectangle(195, 100, 10, 700, rendererConnect4);
    createFilledRectangle(295, 100, 10, 700, rendererConnect4);
    createFilledRectangle(395, 100, 10, 700, rendererConnect4);
    createFilledRectangle(495, 100, 10, 700, rendererConnect4);
    createFilledRectangle(595, 100, 10, 700, rendererConnect4);

    if(actualDisplayedInfo!=NULL){
        createTextZone(rendererConnect4,actualDisplayedInfo,200,10,0,150,0);
    }
    if(actualDisplayedFeedback!=NULL){
        createTextZone(rendererConnect4,actualDisplayedFeedback,200,50,150,0,0);
    }


    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            createSymbolCircle(game[i][j], i, j);
        }
    }
    updateRenderer(rendererConnect4);
}


void initializeGame(char **game) {
    for (int i = 0; i < SIZE; i++) {
        game[i] = malloc(sizeof(char) * SIZE);
        if(game[i]==NULL){
            SDL_ExitWithError("ERROR ALLOCATING game[i]");
        }
        for (int j = 0; j < SIZE; j++) {
            game[i][j] = '+';
        }
    }
}
