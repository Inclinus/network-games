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
int putOnColumn(char **game, int line, char color, int *win);
void initializeGame(char **game);
void displayGameBoard(char **game);

int SIZE = 7;

SDL_Window *windowConnect4 = NULL;
SDL_Renderer *rendererConnect4 = NULL;

int WINDOW_WIDTH = 0;
int WINDOW_HEIGHT = 0;

char red = 'R';
char blue = 'B';

// SDL Bool to do the game loop
SDL_bool connect_launched = SDL_TRUE;


/*
 * CONNECT 4
 * Envoyé par le serveur :
 * - YOURTURN ou WAITTURN
 * - COUP JOUE PAR L ADVERSAIRE
 * - YOUWIN!! ou YOULOSE!
 * Ecouté par le serveur :
 * - PLAY%d par le joueur qui joue
 *
 * Envoyé par le client :
 * - PLAY%d pour la position où il a joué
 * Ecouté par le client :
 * - YOURTURN ou WAITTURN
 * - COUP JOUE PAR L ADVERSAIRE
 * - YOUWIN!! ou YOULOSE!
 */

int * connect4ClientSocket;

SDL_bool tryPlay(char **game, int line, char color) {
    if (line < 1 || line > 7) return SDL_FALSE;
    int count = SIZE - 1;
    while (game[line - 1][count] != '+') {
        count--;
    }
    if (count < 0) return SDL_FALSE;
    game[line - 1][count] = color;
    SDL_Log("[DEBUG] You placed your token on column %d, he fell to the line %d.\n", line, count + 1);
    return SDL_TRUE;
}

int calculateBoardColumnPos(Sint32 x){
    if(x>600){
        return 6;
    } else if(x>500){
        return 5;
    } else if(x>400){
        return 4;
    } else if(x>300){
        return 3;
    } else if(x>200){
        return 2;
    } else if(x>100){
        return 1;
    } else {
        return 0;
    }
}

void *sdlListener() {
    while (connect_launched) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    SDL_Log("SDL QUIT");
                    connect_launched = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int column = calculateBoardColumnPos(event.button.x);
                    SDL_Log("SDL BTN DOWN");
                    NG_Event *buttonDown = malloc(sizeof(NG_Event));
                    buttonDown->type = SDL;
                    buttonDown->instructions = malloc(sizeof(char)*11);
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
    disconnectEvent->type = NETWORK;
    disconnectEvent->instructions = malloc(sizeof(char)*12);
    disconnectEvent->instructions = "DISCONNECTED";

    int player = 1;
    while (connect_launched) {
        char data[9];
        memset(data, '\0', sizeof(data));
        if (recv(*connect4ClientSocket, data, 8, 0) <= 0) {
            sendEvent(disconnectEvent);
            break;
        } else {
            if (strcmp("WAITTURN", data) == 0) {
                NG_Event *enemyTurnEvent = malloc(sizeof(NG_Event));
                enemyTurnEvent->type = NETWORK;
                disconnectEvent->instructions = malloc(sizeof(char)*10);
                enemyTurnEvent->instructions = "ENEMYTURN";
                sendEvent(enemyTurnEvent);
                int enemyPos;
                recv(*connect4ClientSocket, &enemyPos, sizeof(enemyPos), 0);
                NG_Event *enemyPosEvent = malloc(sizeof(NG_Event));
                enemyPosEvent->type = NETWORK;
                enemyPosEvent->instructions = malloc(sizeof(char)*4);
                sprintf(enemyPosEvent->instructions, "ENEMY%d", enemyPos);
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

int connect4(int * socketClient) {
    connect4ClientSocket = socketClient;
    initSDL();
    windowConnect4 = SDL_CreateWindow("CONNECT 4", 50, 50, 700, 800, 0);
    rendererConnect4 = SDL_CreateRenderer(windowConnect4, -1, 0);

    pthread_t network_listener;
    pthread_create(&network_listener, NULL, networkListener, NULL);
    pthread_t sdl_listener;
    pthread_create(&sdl_listener, NULL, sdlListener, NULL);

    char **game = malloc(sizeof(char *) * SIZE);

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

    SDL_bool yourTurn = SDL_FALSE;

    while (connect_launched) {
        NG_Event * event = NULL;
        while ((event=listenAllEvents()) != NULL) {
            SDL_Log("CONNECT4 : EVENT RECEIVED");
            if (event->type == SDL) {
                SDL_Log("CONNECT4 : SDL EVENT RECEIVED %s",event->instructions);
                if (regexec(&posRegex, event->instructions, 0, NULL, 0) == 0) {
                    if(yourTurn==SDL_TRUE){
                        int column;
                        sscanf(event->instructions, "POS%d", &column);
                        if(tryPlay(game,column,'B')){
                            SDL_Log("Coup Possible ! \n");
                            send(*socketClient, &column, sizeof(column), 0);
                            displayGameBoard(game);
                            yourTurn = SDL_FALSE;
                        } else {
                            SDL_Log("Coup Impossible ! \n");
                        }
                    } else {
                        SDL_Log("Ce n'est pas votre tour ! \n");
                    }
                } else {
                    SDL_Log("SDL EVENT RECEIVED %s DOES NOT MATCH REGEX",event->instructions);
                }
            } else if (event->type == NETWORK) {
                SDL_Log("CONNECT4 : NETWORK EVENT RECEIVED %s",event->instructions);
                if (strcmp(event->instructions, "DISCONNECTED") == 0) {
                    SDL_Log("DECONNECTE DU SERVEUR");
                    connect_launched = SDL_FALSE;
                } else if (strcmp("YOURTURN", event->instructions) == 0) {
                    SDL_Log("C'est à vous de jouer ! \n");
                    yourTurn = SDL_TRUE;
                } else if (strcmp("ENEMYTURN", event->instructions) == 0) {
                    SDL_Log("C'est au tour de l'adversaire ! \n");
                    yourTurn = SDL_FALSE;
                } else if (regexec(&enemyPosRegex, event->instructions, 0, NULL, 0) == 0) {
                    int y;
                    sscanf(event->instructions, "ENEMY%d", &y);
                    tryPlay(game,y,'R');
                    displayGameBoard(game);
                } else if (strcmp("YOUWIN!!", event->instructions) == 0) {
                    SDL_Log("Vous avez gagné ! \n");
                    connect_launched = SDL_FALSE;
                } else if (strcmp("YOULOSE!", event->instructions) == 0) {
                    SDL_Log("Vous avez perdu ! \n");
                    connect_launched = SDL_FALSE;
                } else if (strcmp("EQUALITY", event->instructions) == 0) {
                    SDL_Log("Personne n'a gagné ! \n");
                    connect_launched = SDL_FALSE;
                } else {
                    fprintf(stderr,"WTF IS THAT NETWORK EVENT : %s",event->instructions);
                }
            }
        }
    }
    quitSDL(rendererConnect4, windowConnect4);
    close(*socketClient);
    return 0;
}

void createSymbolCircle(char symbol, int x, int y){
    int centerX = 50 + x * (650 - 50) / 2;
    int centerY = 150 + y * (750 - 150) / 2;
    if(symbol=='B'){
        changeColor(rendererConnect4,0,0,255);
        createCircle(rendererConnect4,centerX,centerY,30);
    } else if(symbol=='R'){
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
        for (int j = 0; j < SIZE; j++) {
            game[i][j] = '+';
        }
    }
}
