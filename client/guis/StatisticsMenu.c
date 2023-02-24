#include <sys/socket.h>
#include "StatisticsMenu.h"
#include "MainMenu.h"

SDL_bool * statisticsMenuRunning = NULL;

void statisticsMenu(const int * statisticsSocketClient,SDL_Renderer * rendererMenu, Stats * stats){
    SDL_Log("ENTERING STATISTICS MENU");
    char startData[6];
    memset(startData, '\0', sizeof(startData));
    do{
        if (recv(*statisticsSocketClient, startData, 5, 0) <= 0) {
            SDL_ExitWithError("DISCONNECTED");
            break;
        } else {
            if(strcmp("STAR", startData) != 0){
                SDL_Log("PACKET RECEIVED BUT NOT \"STAR\" : SENDING PONG");
                send(*statisticsSocketClient,"PONG",4,0);
            }

        }
        SDL_Log("ITERATION");
    } while(strcmp("STAR", startData) != 0);

    statisticsMenuRunning = malloc(sizeof(SDL_bool));
    if(statisticsMenuRunning==NULL){
        SDL_ExitWithError("ERROR ALLOCATING CREDITMENURUNNING SDLBOOL");
    }
    *statisticsMenuRunning = SDL_TRUE;

    Button option;
    option.beginX = 20;
    option.beginY = 430;
    option.endX = 150;
    option.endY = 470;

    SDL_RenderClear(rendererMenu);

    createTextZoneCentered(rendererMenu, "Statistics : ",720/2,50, 255, 255, 255,48);

    char * connect4WinText = malloc(sizeof(char)*20);
    if (connect4WinText == NULL) {
        SDL_ExitWithError("ERROR ALLOCATING CONNECT4WINTEXT");
    }
    char * connect4LoseText = malloc(sizeof(char)*20);
    if (connect4LoseText == NULL) {
        SDL_ExitWithError("ERROR ALLOCATING CONNECT4LOSETEXT");
    }
    char * connect4DrawText = malloc(sizeof(char)*20);
    if (connect4DrawText == NULL) {
        SDL_ExitWithError("ERROR ALLOCATING CONNECT4DRAWTEXT");
    }
    char * ticTacToeWinText = malloc(sizeof(char)*20);
    if (ticTacToeWinText == NULL) {
        SDL_ExitWithError("ERROR ALLOCATING TICTACTOEWINTEXT");
    }
    char * ticTacToeLoseText = malloc(sizeof(char)*20);
    if (ticTacToeLoseText == NULL) {
        SDL_ExitWithError("ERROR ALLOCATING TICTACTOELOSETEXT");
    }
    char * ticTacToeDrawText = malloc(sizeof(char)*20);
    if (ticTacToeDrawText == NULL) {
        SDL_ExitWithError("ERROR ALLOCATING TICTACTOEDRAWTEXT");
    }

    sprintf(connect4WinText,"Connect 4 Win : %d", stats->nbWinConnect4);
    sprintf(connect4LoseText,"Connect 4 Lose : %d", stats->nbLooseConnect4);
    sprintf(connect4DrawText,"%s %d", "Connect 4 Draw :", stats->nbDrawConnect4);
    sprintf(ticTacToeWinText,"%s %d", "TicTacToe Win :", stats->nbWinTictactoe);
    sprintf(ticTacToeLoseText,"%s %d", "TicTacToe Lose :", stats->nbLooseTictactoe);
    sprintf(ticTacToeDrawText,"%s %d", "TicTacToe Draw :", stats->nbDrawTictactoe);

    createTextZoneCentered(rendererMenu,connect4WinText,720/2,120, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,connect4LoseText,720/2,160, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,connect4DrawText,720/2,200, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,ticTacToeWinText,720/2,240, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,ticTacToeLoseText,720/2,280, 255,255,255, 24);
    createTextZoneCentered(rendererMenu,ticTacToeDrawText,720/2,320, 255,255,255, 24);

    createButton(rendererMenu,option, "retour");

    updateRenderer(rendererMenu);


    while(*statisticsMenuRunning){
        SDL_Event event;

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    *statisticsMenuRunning = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>option.beginX && x<option.endX && y<option.endY && y>option.beginY){
                        *statisticsMenuRunning = SDL_FALSE;
                        loadMainMenu();
                    }
                    break;
                default:
                    break;
            }
        }
    }
} 
