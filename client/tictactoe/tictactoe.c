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


void printboard(int **board);

void placeSymbol(int ** board, int symbol, int px, int py);

void yourTurn(int socketClient);

SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;

SDL_bool program_launched = SDL_TRUE;
int clientSocket;


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
                    SDL_Log("SDL BTN DOWN");
                    NG_Event *buttonDown = malloc(sizeof(NG_Event));
                    buttonDown->type = SDL;
                    buttonDown->instructions = "BUTTONDOWN";
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
    disconnectEvent->instructions = "DISCONNECT";

    char data[8];
    int player = 1;
    while (program_launched) {
        if (recv(clientSocket, data, 8, 0) <= 0) {
            sendEvent(disconnectEvent);
            break;
        } else {
            if (strcmp("WAITTURN", data) == 0) {
                NG_Event *enemyTurnEvent;
                enemyTurnEvent->type = NETWORK;
                enemyTurnEvent->instructions = "ENEMYTURN";
                sendEvent(enemyTurnEvent);
                int px;
                int py;
                recv(clientSocket, &px, sizeof(px), 0);
                recv(clientSocket, &py, sizeof(py), 0);
                NG_Event *enemyPosEvent;
                enemyPosEvent->type = NETWORK;
                sprintf(enemyPosEvent->instructions, "%d-%d", px, py);
                sendEvent(enemyPosEvent);
            } else {
                NG_Event *receivedDataEvent;
                receivedDataEvent->type = NETWORK;
                receivedDataEvent->instructions = data;
                sendEvent(receivedDataEvent);
            }
        }
        // 2-0



        data[0] = '\0';
    }


}

int tictactoe(int socketClient) {
    clientSocket = socketClient;
    initSDL();
    window = SDL_CreateWindow("MORPION", 50, 50, 600, 600, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);

    pthread_t network_listener;
    pthread_create(&network_listener, NULL, networkListen, NULL);
    pthread_t sdl_listener;
    pthread_create(&sdl_listener, NULL, sdlListen, NULL);

    changeColor(renderer, 255, 255, 255);
    createFilledRectangle(0, 0, 600, 600, renderer);
    changeColor(renderer, 0, 0, 0);
    createFilledRectangle(0, 195, 600, 10, renderer);
    createFilledRectangle(0, 395, 600, 10, renderer);
    createFilledRectangle(195, 0, 10, 600, renderer);
    createFilledRectangle(395, 0, 10, 600, renderer);
    updateRenderer(renderer);

    // TODO create board in SDL, we don't need the board
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
            SDL_Log("| %d ", board[i][y]);
        }
        SDL_Log("| \n");
    }

    regex_t posRegex;
    if (regcomp(&posRegex, "^[0-9]-[0-9]$", REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Error: Could not compile regular expression\n");
        return 1;
    }

    regex_t enemyPosRegex;
    if (regcomp(&posRegex, "^YES[0-9]-[0-9]$", REG_EXTENDED | REG_NOSUB) != 0) {
        fprintf(stderr, "Error: Could not compile regular expression\n");
        return 1;
    }

    int swap = 0;

    while (program_launched) {
        NG_Event event;
        while (listenAllEvents(&event)) {
            if (event.type == SDL) {
                if (strcmp(event.instructions, "BUTTONDOWN") == 0) {
                    SDL_Log("SDL button down %d", swap);
                    if (swap % 2 == 0) {
                        changeColor(renderer, 255, 0, 0);
                    } else {
                        changeColor(renderer, 0, 0, 255);
                    }
                    swap++;
                    createFilledRectangle(50, 50, 200, 120, renderer);
                    updateRenderer(renderer);
                }
            } else if (event.type == NETWORK) {
                if (strcmp(event.instructions, "DISCONNECTED") == 0) {
                    SDL_Log("DECONNECTE DU SERVEUR");
                    program_launched = SDL_FALSE;
                } else if (strcmp("YOURTURN", event.instructions) == 0) {
                    yourTurn(socketClient);
                } else if (strcmp("ENEMYTURN", event.instructions) == 0) {
                    SDL_Log("C'est au tour de l'adversaire ! \n");
                } else if (regexec(&enemyPosRegex, event.instructions, 0, NULL, 0) == 0) {
                    int x, y;
                    sscanf(event.instructions, "%d-%d", &x, &y);
                    placeSymbol(board, 2, x, y);
                } else if (strcmp("NOK", event.instructions) == 0) {
                    SDL_Log("Coup Impossible ! \n");
                    yourTurn(socketClient);
                } else if (regexec(&posRegex, event.instructions, 0, NULL, 0) == 0) {
                    SDL_Log("Coup Possible ! \n");
                    int x, y;
                    sscanf(event.instructions, "YES%d-%d", &x, &y);
                    placeSymbol(board, 1, x, y);
                } else if (strcmp("YOUWIN!!", event.instructions) == 0) {
                    SDL_Log("Vous avez gagné ! \n");
                    program_launched = SDL_FALSE;
                } else if (strcmp("YOULOSE!", event.instructions) == 0) {
                    SDL_Log("Vous avez perdu ! \n");
                    program_launched = SDL_FALSE;
                } else {
                    fprintf(stderr,"WTF IS THAT NETWORK EVENT : %s",event.instructions);
                }
            }
        }
    }
    quitSDL(renderer, window);
    close(socketClient);
    return 0;
}

void yourTurn(int socketClient) {
    int px;
    int py;
    printf("C'est a vous de jouer ! \n");
    scanf("%d", &px);
    send(socketClient, &px, sizeof(px), 0);
    scanf("%d", &py);
    send(socketClient, &py, sizeof(py), 0);
}

void printboard(int **board) {
    for (int i = 2; i >= 0; --i) {
        for (int y = 0; y < 3; ++y) {
            printf("| %d ", board[i][y]);
        }
        printf("| \n");
    }
}

void placeSymbol(int **board, int symbol, int px, int py) {
    board[px][py] = symbol;
}
