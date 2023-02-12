#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "tictactoe/tictactoe.h"
#include "connect4/connect4.h"
#include "bdd/database.h"

typedef struct GameArgs {
    int socketPlayer1;
    int socketPlayer2;
    int GameId;
} GameArgs;

void * startGame(void *args){
    struct GameArgs *myargs;
    myargs = (struct GameArgs *) args;

    printf("La game id %d commence !\n",((GameArgs *)args)->GameId);

    char data[25];
    char data2[25];

    send(myargs->socketPlayer1, "NICKNAME", 8, 0);
    send(myargs->socketPlayer2, "NICKNAME", 8, 0);

    recv(myargs->socketPlayer1, data, sizeof(data), 0);
    recv(myargs->socketPlayer2, data2, sizeof(data), 0);

    printf("NICKNAME 1 : %s\n", data);
    printf("NICKNAME 2 : %s\n", data2);


    tictactoe(myargs->socketPlayer1, myargs->socketPlayer2);
    //connect4Server(myargs->socketPlayer1, myargs->socketPlayer2);

    return 0;
}

void * login(int * socketClient){

    MYSQL *con = connectBdd();

    if (con == NULL) {
        exit(1);
    }

    int flag = 1;

    while(flag == 1) {
        printf("En attente de la demande de login ...\n");
        char respons[9];
        recv(*socketClient, respons, sizeof(respons), 0);
        respons[8] = '\0';
        printf("RECU : %s\n", respons);

        char buffer_login[25];
        char buffer_password[25];
        recv(*socketClient, buffer_login, sizeof(buffer_login), 0);
        printf("RECU : %s\n", buffer_login);
        recv(*socketClient, buffer_password, sizeof(buffer_password), 0);
        printf("RECU : %s\n", buffer_password);

        if (strcmp(respons, "LOGINCLI") == 0) {
            if (checkUser(con, buffer_login, buffer_password) == 1) {
                printf("L'utilisateur existe et le mot de passe sont correct\n");
                send(*socketClient, "OK", 2, 0);
                flag = 0;
            } else {
                printf("L'utilisateur n'existe pas ou le mot de passe est incorrect\n");
                send(*socketClient, "KO", 2, 0);
                flag = 1;
            }
        } else if (strcmp(respons, "REGISTER") == 0){
            if (createUser(con, buffer_login, buffer_password) == 0) {
                printf("L'utilisateur a bien été créer\n");
                send(*socketClient, "OK", 2, 0);
                flag = 0;
            } else {
                printf("L'utilisateur n'a pas pu être créer\n");
                send(*socketClient, "KO", 2, 0);
                flag = 1;
            }
        } else {
            printf("WTF THIS PACKET !\n");
        }
    }
}

int main() {

    int socketServer = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrServer;
    addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    //addrServer.sin_addr.s_addr = inet_addr("92.222.131.57");
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(4444);

    if (bind(socketServer, (struct sockaddr *)&addrServer, sizeof(addrServer)) < 0) {
        printf("BIND SOCKET ERREUR !\n");
        return 1;
    }
    printf("BIND SOCKET OK !\n");

    struct sockaddr_in addrClient;
    socklen_t addrClient_lenght;

    int nbJoueur = 0;
    int maxJoueur = 255;
    int *socketClient = (int*)malloc(maxJoueur*sizeof(int));
    int GameId = 0;

    // TEST BDD
//    MYSQL *con = connectBdd();
//
//    if (con == NULL) {
//        exit(1);
//    }
//
//    if (createUser(con, "tibo", "mdpdeouf") == 0) {
//        printf("L'utilisateur a bien été créer\n");
//    } else {
//        printf("L'utilisateur n'a pas pu être créer\n");
//    }

//    if(checkUser(con, "tibo", "mdpdeouf") == 0) {
//        printf("L'utilisateur existe et le mot de passe sont correct\n");
//    } else {
//        printf("L'utilisateur n'existe pas ou le mot de passe est incorrect\n");
//    }

    //closeBdd(con);
    // FIN TEST BDD

    while (nbJoueur < maxJoueur){
        listen(socketServer, maxJoueur - nbJoueur);
        addrClient_lenght = sizeof(addrClient);
        socketClient[nbJoueur] = accept(socketServer, (struct sockaddr *) &addrClient, &addrClient_lenght);

        if (socketClient[nbJoueur] < 0) {
            printf("ERREUR DE CONNEXION AVEC LE CLIENT !\n");
        }
        printf("CONNEXION ACCEPTER DU CLIENT NUMERO :  %d\n", nbJoueur);

        nbJoueur++;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, (void*)login, &socketClient[nbJoueur-1]);
//        GameArgs args;
//        args.socketPlayer1 = socketClient[nbJoueur-2];
//        args.socketPlayer2 = socketClient[nbJoueur-1];
//        args.GameId = GameId;
//        pthread_t thread_id;
//        pthread_create(&thread_id, NULL, startGame, &args);
//        GameId++;
    }


    //pthread_join(thread_id, NULL);

    // Close Socket client
    for (int i = 0; i < nbJoueur; ++i) {
        close(socketClient[i]);
    }
    // Close Socket server
    close(socketServer);

    pthread_exit(NULL);
    return 0;
}
