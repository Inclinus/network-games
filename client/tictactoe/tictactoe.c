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
#include "../../sdl-utils/SDLUtils.h"
#include "../../events/EventManager.h"


void printboard(int ** board);
int turn(int ** board, int player, int px, int py);

SDL_Renderer * renderer = NULL;
SDL_Window * window = NULL;

SDL_bool program_launched = SDL_TRUE;
int clientSocket;


void * sdlListen(){
    while(program_launched){
        SDL_Event event;
        //SDL_Log("BOUCLE DE JEU");

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    SDL_Log("SDL QUIT");
                    program_launched = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    SDL_Log("SDL BTN DOWN");
                    NG_Event * buttonDown = malloc(sizeof(NG_Event));
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

void * networkListen(){
    NG_Event * disconnectEvent = malloc(sizeof(NG_Event));
    disconnectEvent->type = NETWORK;
    disconnectEvent->instructions = "DISCONNECT";

    char data[8];
    int player=1;
    while (program_launched) {
        if (recv(clientSocket, data, 8, 0) <= 0){
            sendEvent(disconnectEvent);
            break;
        }
        NG_Event receivedDataEvent;
        receivedDataEvent.type = NETWORK;
        receivedDataEvent.instructions = data;
    }



}

int tictactoe(int socketClient) {
    clientSocket = socketClient;
    initSDL();
    window = SDL_CreateWindow("MORPION",50,50,600,600,0);
    renderer = SDL_CreateRenderer(window,-1,0);

    pthread_t network_listener;
    pthread_create(&network_listener, NULL, networkListen, NULL);
    pthread_t sdl_listener;
    pthread_create(&sdl_listener,NULL,sdlListen,NULL);

    changeColor(renderer,255,255,255);
    createFilledRectangle(0,0,600,600,renderer);
    changeColor(renderer,0,0,0);
    createFilledRectangle(0,195,600,10,renderer);
    createFilledRectangle(0,395,600,10,renderer);
    createFilledRectangle(195,0,10,600,renderer);
    createFilledRectangle(395,0,10,600,renderer);
    updateRenderer(renderer);

    // TODO create board in SDL, we don't need the board
//    int ** board;
//    int * row;
//    board=malloc(sizeof(int*)*3);
//
//    for (int i = 0; i < 3; ++i)
//    {
//        row=malloc(sizeof(int)*3);
//        board[i]=row;
//
//    }
//    for (int i = 0; i < 3; ++i)
//    {
//        for (int y = 0; y < 3; ++y)
//        {
//            board[i][y]=0;
//            SDL_Log("| %d ",board[i][y]);
//        }
//        SDL_Log("| \n");
//    }


    int swap = 0;

    while (program_launched){
        NG_Event event;
        while (listenAllEvents(&event)){
            if(event.type==SDL){
                if(strcmp(event.instructions,"BUTTONDOWN")==0) {
                    SDL_Log("SDL button down %d",swap);
                    if(swap%2==0) {
                        changeColor(renderer,255,0,0);
                    }
                    else {
                        changeColor(renderer,0,0,255);
                    }
                    swap++;
                    createFilledRectangle(50,50,200,120,renderer);
                    updateRenderer(renderer);
                }
            }
//            else if(event.type == NETWORK){
//                if(strcmp(event.instructions,"DISCONNECTED") == 0){
//                    SDL_Log("DECONNECTE DU SERVEUR");
//                } else {
//
//                    //printf("RECU : %s\n", data);
//
//                    player+=1;
//                    printboard(board);
//                    if (strcmp("YOURTURN", event.instructions) == 0) {
//                        int px;
//                        int py;
//                        printf("C'est a vous de jouer ! \n");
//                        scanf("%d", &px);
//                        send(socketClient, &px, sizeof(px), 0);
//                        scanf("%d", &py);
//                        send(socketClient, &py, sizeof(py), 0);
//                        char data2[3];
//                        if (recv(socketClient, data2, 3, 0) <= 0){
//                            sendEvent(disconnectEvent);
//                            break;
//                        }
//                        printf("RECU2 : %s\n", data2);
//                        if(strstr(data2, "NOK")!= NULL){
//                            printf("Coup Impossible ! \n");
//                        }else{
//                            printf("Coup Possible ! \n");
//                            turn(board,player,px,py);
//                        }
//                        event.instructions[0] = '\0';
//                    } else if (strcmp("WAITTURN", event.instructions) == 0) {
//                        printf("C'est au tour de l'adversaire ! \n");
//                        int px;
//                        int py;
//                        recv(socketClient, &px, sizeof(px), 0);
//                        recv(socketClient, &py, sizeof(py), 0);
//                        turn(board,player,px,py);
//                        event.instructions[0] = '\0';
//                    } else if (strcmp("YOUWIN!!", event.instructions) == 0) {
//                        printf("Vous avez gagné ! \n");
//                        program_launched = SDL_FALSE;
//                    } else if (strcmp("YOULOSE!", event.instructions) == 0) {
//                        printf("Vous avez perdu ! \n");
//                        program_launched = SDL_FALSE;
//                    }
//                }
//            }
        }
    }
    quitSDL(renderer,window);
    close(socketClient);
    return 0;
}

void printboard(int ** board) {
    for (int i = 2; i >=0; --i)
    {
        for (int y = 0; y < 3; ++y)
        {
            printf("| %d ",board[i][y]);
        }
        printf("| \n");
    }
}

int turn(int ** board, int player, int px, int py)
{
    if(player%2==0 && board[px][py]!=1 && board[px][py]!=2){
        board[px][py]=1;
        return 1;
    }
    else if(player%2==1 && board[px][py]!=1 && board[px][py]!=2){
        board[px][py]=2;
        return 1;
    }else{
        printf("Coup Impossible ! \n");
        printboard(board);
        return 0;
    }
}
