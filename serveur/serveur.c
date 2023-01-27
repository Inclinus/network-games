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

int main() {

    int socketServer = socket(AF_INET, SOCK_STREAM, 0);
    // GERER L'ERREUR SI SOCKET NE S'OUVRE PAS !
    // MALLOC ?
    struct sockaddr_in addrServer;
    addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(4444);

    if (bind(socketServer, (const struct sockaddr_in *)&addrServer, sizeof(addrServer)) < 0) {
        printf("BIND SOCKET ERREUR !\n");
    }
    printf("BIND SOCKET OK !\n");

    struct sockaddr_in addrClient;
    socklen_t addrClient_lenght;

    int nbJoueur = 0;
    int maxJoueur = 2;
    int *socketClient = (int*)malloc(maxJoueur*sizeof(int));

    while (nbJoueur < maxJoueur){

        listen(socketServer, maxJoueur - nbJoueur);
        addrClient_lenght = sizeof(addrClient);
        socketClient[nbJoueur] = accept(socketServer, (struct sockaddr *) &addrClient, &addrClient_lenght);

        if (socketClient[nbJoueur] < 0) {
            printf("ERREUR DE CONNEXION AVEC LE CLIENT !\n");
        }
        printf("CONNEXION ACCEPTER DU CLIENT NUMERO :  %d\n", nbJoueur);
        send(socketClient[nbJoueur], "NICKNAME", 8, 0);
        char data[25];
        recvfrom(socketClient[nbJoueur], data, sizeof(data), 0, (struct sockaddr *) &addrClient, &addrClient_lenght);
        printf("NICKNAME : %s\n", data);
        nbJoueur++;
    }

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
            printf("C'est au tour du joueur 1 !\n");
            send(socketClient[0], "YOURTURN", 8, 0);
            send(socketClient[1], "WAITTURN", 8, 0);
            int px;
            int py;
            recv(socketClient[0], &px, sizeof(px), 0);
            recv(socketClient[0], &py, sizeof(py), 0);
            if(turn(board,player,px,py)){
                printf("Coup POSSIBLE SEND TO CLIENT ! \n");
                send(socketClient[0], "YES", 3, 0);
                send(socketClient[1], &px, sizeof(px), 0);
                send(socketClient[1], &py, sizeof(py), 0);
            } else {
                printf("Coup IMPOSSIBLE SEND TO CLIENT ! \n");
                send(socketClient[0], "NOK", 3, 0);
            }
        } else if (player%2==1){
            printf("C'est au tour du joueur 2 !\n");
            send(socketClient[0], "WAITTURN", 8, 0);
            send(socketClient[1], "YOURTURN", 8, 0);
            int px;
            int py;
            recv(socketClient[1], &px, sizeof(px), 0);
            recv(socketClient[1], &py, sizeof(py), 0);
            if(turn(board,player,px,py)){
                printf("Coup POSSIBLE SEND TO CLIENT ! \n");
                send(socketClient[1], "YES", 3, 0);
                send(socketClient[0], &px, sizeof(px), 0);
                send(socketClient[0], &py, sizeof(py), 0);
            } else {
                printf("Coup IMPOSSIBLE SEND TO CLIENT ! \n");
                send(socketClient[1], "NOK", 3, 0);
            }
        }

        if (winCondition(board,1)){
            printf("Le joueur 1 a gagné !\n");
            send(socketClient[0], "YOUWIN!!", 8, 0);
            send(socketClient[1], "YOULOSE!", 8, 0);
            flag=1;
        }else if (winCondition(board,2)){
            printf("Le joueur 2 a gagné !\n");
            send(socketClient[0], "YOULOSE!", 8, 0);
            send(socketClient[1], "YOUWIN!!", 8, 0);
            flag=1;
        }

    }
    free(row);
    free(board);


    // Close Socket client
    close(socketClient[0]);
    close(socketClient[1]);
    // Close Socket server
    close(socketServer);

    return 0;
}
