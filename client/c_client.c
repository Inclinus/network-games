#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

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
    int socketClient = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrServer;
    // gethostbyname() -> permet d'utilise le DNS
    addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(4444);

    connect(socketClient, (const struct sockaddr *) &addrServer, sizeof(addrServer));
    printf("CONNECTER !\n");

    char data[8];
    recv(socketClient, data, 8, 0);
    printf("RECU : %s\n", data);
    if (strcmp(data, "NICKNAME")) {
        printf("Quel est votre pseudo : \n");
        scanf("%s", data);
        send(socketClient, data, sizeof(data), 0);
    } else {
        printf("ERREUR DE PROTOCOLE !\n");
    }




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
    int JENECOMPRENDPAS = 0;

    while (flag==0) {
        if (recv(socketClient, data, 8, 0) <= 0){
            printf("Deconnecter du serveur ! \n");
            break;
        }
        if (JENECOMPRENDPAS == 0) {
            data[strlen(data)-1] = '\0';
            JENECOMPRENDPAS++;
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