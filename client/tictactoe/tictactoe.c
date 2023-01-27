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
#include "../../sdl-utils/SDLUtils.h"


void printboard(int ** board);
int turn(int ** board, int player, int px, int py);

SDL_Renderer * renderer = NULL;
SDL_Window * window = NULL;

SDL_bool program_launch = SDL_TRUE;

typedef struct{
    int beginX;
    int beginY;
    int endX;
    int endY;
    int actionType;
} Button ;

int tictactoe(int socketClient) {

    initSDL();
    window = SDL_CreateWindow("MORPION",50,50,400,400,0);
    renderer = SDL_CreateRenderer(window,-1,0);


    Button btn;
    btn.beginX = 50;
    btn.beginY = 50;
    btn.endX = 250;
    btn.endY = 150;
    btn.actionType = 6;

    changeColor(renderer,0,255,0);
    createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,renderer);
    updateRenderer(renderer);

    while(program_launch){
        SDL_Event event;


        int swap = 0;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    program_launch = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    printf("SDL BUTTON DOWN");
                    if(event.button.button==SDL_BUTTON_LEFT){
                        if(swap%2==0) {
                            changeColor(renderer,255,0,0);
                        }
                        else {
                            changeColor(renderer,0,0,255);
                        }

                        swap++;
                        createFilledRectangle(btn.beginX,btn.beginY,btn.endX-btn.beginX,btn.endY-btn.beginY,renderer);
                        updateRenderer(renderer);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    char data[8];
    int ** board;
    int * row;
    board=malloc(sizeof(int*)*3);

    for (int i = 0; i < 3; ++i)
    {
        row=malloc(sizeof(int)*3);
        board[i]=row;

    }
    for (int i = 0; i < 3; ++i)
    {
        for (int y = 0; y < 3; ++y)
        {

            board[i][y]=0;
            printf("| %d ",board[i][y]);
        }
        printf("| \n");
    }
    int player=1;
    int flag=0;

    while (flag==0) {
        if (recv(socketClient, data, 8, 0) <= 0){
            printf("Deconnecter du serveur ! \n");
            break;
        }
        printf("RECU : %s\n", data);
        player+=1;
        printboard(board);
        if (strcmp("YOURTURN", data) == 0) {
            int px;
            int py;
            printf("C'est a vous de jouer ! \n");
            scanf("%d", &px);
            send(socketClient, &px, sizeof(px), 0);
            scanf("%d", &py);
            send(socketClient, &py, sizeof(py), 0);
            char data2[3];
            if (recv(socketClient, data2, 3, 0) <= 0){
                printf("Deconnecter du serveur ! \n");
                break;
            }
            printf("RECU2 : %s\n", data2);
            if(strstr(data2, "NOK")!= NULL){
                printf("Coup Impossible ! \n");
            }else{
                printf("Coup Possible ! \n");
                turn(board,player,px,py);
            }
            data[0] = '\0';
        } else if (strcmp("WAITTURN", data) == 0) {
            printf("C'est au tour de l'adversaire ! \n");
            int px;
            int py;
            recv(socketClient, &px, sizeof(px), 0);
            recv(socketClient, &py, sizeof(py), 0);
            turn(board,player,px,py);
            data[0] = '\0';
        } else if (strcmp("YOUWIN!!", data) == 0) {
            printf("Vous avez gagné ! \n");
            flag=1;
        } else if (strcmp("YOULOSE!", data) == 0) {
            printf("Vous avez perdu ! \n");
            flag=1;
        }
    }

    free(row);
    free(board);

    //printf("STRING RECU : %s\n", buffer);
    close(socketClient);

    return 0;
}

void printboard(int ** board)
{
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
