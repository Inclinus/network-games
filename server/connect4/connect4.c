#include <stdio.h>
#include <stdlib.h>


int connect4(int socketPlayer1, int socketPlayer2) {

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
            processPlayerTurn(1,socketPlayer1, socketPlayer2, board, player,&flag);
        } else if (player%2==1){
            processPlayerTurn(2,socketPlayer2, socketPlayer1, board, player,&flag);
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
