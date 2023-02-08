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

    return 0;
}

int main() {

    int socketServer = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrServer;
    // addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrServer.sin_addr.s_addr = inet_addr("92.222.131.57");
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
    MYSQL *con = connectBdd();

    if (con == NULL) {
        exit(1);
    }

    //createUser(con, "test2", "test2");

    if(checkUser(con, "test", "test1")){
        printf("L'utilisateur existe et le mot de passe sont correct\n");
    } else {
        printf("L'utilisateur n'existe pas ou le mot de passe est incorrect\n");
    }

    closeBdd(con);

    // FIN TEST BDD

    while (nbJoueur < maxJoueur){
        int connected = 0;
        while (connected < 2) {
            listen(socketServer, maxJoueur - nbJoueur);
            addrClient_lenght = sizeof(addrClient);
            socketClient[nbJoueur] = accept(socketServer, (struct sockaddr *) &addrClient, &addrClient_lenght);

            if (socketClient[nbJoueur] < 0) {
                printf("ERREUR DE CONNEXION AVEC LE CLIENT !\n");
            }
            printf("CONNEXION ACCEPTER DU CLIENT NUMERO :  %d\n", nbJoueur);

            connected++;
            nbJoueur++;
        }
        GameArgs args;
        args.socketPlayer1 = socketClient[nbJoueur-2];
        args.socketPlayer2 = socketClient[nbJoueur-1];
        args.GameId = GameId;
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, startGame, &args);
        GameId++;
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
