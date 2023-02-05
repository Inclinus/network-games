#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

struct Player {
    char *nickname;
    int id;
};

void processPlayerTurn(int playerID, int socketPlayer1, int socketPlayer2, int **board, int player);

int win(int winner, int loser, int player);

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
int winCondition(int ** board,int verifNumber){

    for (int i = 0; i < 2; i++) {
        if (board[i][0] == verifNumber && board[i][1] == verifNumber && board[i][2] == verifNumber) {
            return 1;
        }
    }

    for (int y = 0; y < 2; y++) {
        if (board[0][y] == verifNumber && board[1][y] == verifNumber && board[2][y] == verifNumber) {
            return 1;
        }
    }

    if (board[0][0] == verifNumber && board[1][1] == verifNumber && board[2][2] == verifNumber) {
        return 1;
    }
    if (board[0][2] == verifNumber && board[1][1] == verifNumber && board[2][0] == verifNumber) {
        return 1;
    }

    return 0;
}

int tictactoe(int socketPlayer1, int socketPlayer2) {

    printf("PARTIE COMMENCE !\n");

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
        player+=1;
        printboard(board);

        if (player%2==0){
            processPlayerTurn(1,socketPlayer1, socketPlayer2, board, player);
        } else if (player%2==1){
            processPlayerTurn(2,socketPlayer2, socketPlayer1, board, player);
        }

        if (winCondition(board,1)){
            flag = win(socketPlayer1, socketPlayer2, 1);
        } else if (winCondition(board,2)){
            flag = win(socketPlayer2, socketPlayer1, 2);
        }

    }
    free(row);
    free(board);

    return 0;
}

int win(int winner, int loser, int player) {
    printf("Le joueur %d a gagné !\n",player);
    send(winner, "YOUWIN!!", 8, 0);
    send(loser, "YOULOSE!", 8, 0);
    return 1;
}

void processPlayerTurn(int playerID,int socketPlayer1, int socketPlayer2, int **board, int player) {
    printf("C'est au tour du joueur %d !\n",playerID);
    send(socketPlayer1, "YOURTURN", 8, 0);
    send(socketPlayer2, "WAITTURN", 8, 0);
    int px;
    int py;
    recv(socketPlayer1, &px, sizeof(px), 0);
    recv(socketPlayer1, &py, sizeof(py), 0);
    if(turn(board,player,px,py)){
        printf("Coup POSSIBLE SEND TO CLIENT ! \n");
        char pos[7];
        sprintf(pos,"YES%d-%d",px,py);
        send(socketPlayer1, pos, sizeof(pos), 0);
        send(socketPlayer2, &px, sizeof(px), 0);
        send(socketPlayer2, &py, sizeof(py), 0);
    } else {
        printf("Coup IMPOSSIBLE SEND TO CLIENT ! \n");
        send(socketPlayer1, "NOK", 3, 0);
    }
}
