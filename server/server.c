#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "tictactoe/tictactoe.h"
#include "connect4/connect4.h"
#include "bdd/database.h"

typedef struct GameArgs {
    int socketPlayer1;
    int socketPlayer2;
    int GameId;
} GameArgs;

typedef struct LoginArgs {
    int socketClient;
    int nbJoueur;
} LoginArgs;

GameArgs args;

void * login(void * loginargs);

int lobby(int * socketClient, char * login);

void * startGame(void *args){
    struct GameArgs *myargs;
    myargs = (struct GameArgs *) args;

    printf("La game id %d commence !\n",((GameArgs *)args)->GameId);
    printf("Le joueur 1 est le socket %d\n",((GameArgs *)args)->socketPlayer1);

    while (myargs->socketPlayer1 == 0 || myargs->socketPlayer2 == 0) {
        printf("En attente de joueur ...\n");
        sleep(1);
    }
    printf("Le joueur 2 est le socket %d\n",((GameArgs *)args)->socketPlayer2);

    send(myargs->socketPlayer1, "LOBBYHOST", 9, 0);
    send(myargs->socketPlayer2, "LOBBYJOUR", 9, 0);

    char choix[11];
    recv(myargs->socketPlayer1, choix, sizeof(choix), 0);
    choix[10] = '\0';
    printf("RECU : %s\n", choix);


    if(strcmp(choix, "LEAVEGAME") == 0){
        send(myargs->socketPlayer1, "GAMEBREAK", 9, 0);
        send(myargs->socketPlayer2, "GAMEBREAK", 9, 0);
    } else if (strcmp(choix, "TICTACTOE") == 0) {
        send(myargs->socketPlayer2, "TICTACTOE", 9, 0);
        tictactoe(myargs->socketPlayer1, myargs->socketPlayer2);
    } else if (strcmp(choix, "NCONNECT4") == 0) {
        send(myargs->socketPlayer2, "NCONNECT4", 9, 0);
        connect4Server(myargs->socketPlayer1, myargs->socketPlayer2);
    }
    printf("La game id %d est terminé !\n",((GameArgs *)args)->GameId);
//    lobby(&myargs->socketPlayer1);
//    printf("LOBBY CREATE FOR PLAYERS\n");
//    lobby(&myargs->socketPlayer2);
//    printf("LOBBY CREATE FOR PLAYERS\n");
    return 0;
}

void * login(void * loginargs) {
    ;

    struct LoginArgs *myloginargs;
    myloginargs = (struct LoginArgs *) loginargs;

    MYSQL *con = connectBdd();

    if (con == NULL) {
        exit(1);
    }

    int flag = 1;

    int socketClient = myloginargs->socketClient;

    char buffer_login[25];
    char buffer_password[25];

    while (flag) {
        printf("En attente de la demande de login ...\n");
        char respons[9];
        recv(socketClient, respons, sizeof(respons), 0);
        respons[8] = '\0';
        printf("RECU : %s\n", respons);

        recv(socketClient, buffer_login, sizeof(buffer_login), 0);
        printf("RECU : %s\n", buffer_login);
        recv(socketClient, buffer_password, sizeof(buffer_password), 0);
        printf("RECU : %s\n", buffer_password);

        if (strcmp(respons, "LOGINCLI") == 0) {
            if (connectUser(con, buffer_login, buffer_password) == 1) {
                printf("L'utilisateur existe et le mot de passe sont correct\n");
                send(socketClient, "OK", 2, 0);
                flag = 0;
            } else {
                printf("L'utilisateur n'existe pas ou le mot de passe est incorrect\n");
                send(socketClient, "KO", 2, 0);
                flag = 1;
            }
        } else if (strcmp(respons, "REGISTER") == 0){
            if (createUser(con, buffer_login, buffer_password) == 0) {
                printf("L'utilisateur a bien été créer\n");
                send(socketClient, "OK", 2, 0);
                flag = 0;
            } else {
                printf("L'utilisateur n'a pas pu être créer\n");
                send(socketClient, "KO", 2, 0);
                flag = 1;
            }
        } else {
            printf("WTF THIS PACKET !\n");
            exit(EXIT_FAILURE);
        }
    }
    lobby(&socketClient, buffer_login);
    closeBdd(con);
}

int lobby(int * socketClient, char * login) {
    int flag = 1;
    while (flag) {
        char choix[6];
        recv(*socketClient, choix, sizeof(choix), 0);
        choix[5] = '\0';
        printf("RECU AFTER AUTH 2 : %s\n", choix);

        if (strcmp(choix, "QUEUE") == 0) {
            if (args.socketPlayer1 == 0) {
                printf("CREATION DE LOBBY !\n");
                args.socketPlayer1 = *socketClient;
                args.socketPlayer2 = 0;
                args.GameId = 1;
                pthread_t threadGame;
                pthread_create(&threadGame, NULL, startGame, (void *) &args);
                pthread_join(threadGame, NULL);
            } else {
                printf("JOIN DE LOBBY !\n");
                args.socketPlayer2 = *socketClient;
            }
        } else if (strcmp(choix, "STATS") == 0) {
            // TODO STATS
            send(*socketClient, "STATS", 5, 0);
            MYSQL *con = connectBdd();
            Stats result = getStats(con, login);
            closeBdd(con);
            printf("nbWinTictactoe : %d", result.nbWinTictactoe);
            send(*socketClient, &result, sizeof(result), 0);
            printf("STATS ENVOYER !\n");
        } else {
            printf("WTF THIS PACKET !\n");
            close(*socketClient);
            printf("SOCKET CLOSED !\n");
            flag = 0;
            //exit(EXIT_FAILURE);
        }
    }
}

int main() {

    int socketServer = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrServer;
    //addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrServer.sin_addr.s_addr = inet_addr("0.0.0.0");
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(4444);

    int yes = 1;
    // Met l'option "REUSEADDRESS" pour le socket à "1" pour permettre d'éviter une erreur de bind sur le port après un crash
    if (setsockopt(socketServer, SOL_SOCKET, SO_REUSEADDR,
                   (void*)&yes, sizeof(yes)) < 0) {
        printf( "setsockopt() failed.\n %d",errno);
    }

    if (bind(socketServer, (struct sockaddr *)&addrServer, sizeof(addrServer)) < 0) {
        printf("BIND SOCKET ERREUR !\n ERRNO = %d\n",errno);
        return 1;
    }
    printf("BIND SOCKET OK !\n");

    struct sockaddr_in addrClient;
    socklen_t addrClient_lenght;

    int nbJoueur = 0;
    int maxJoueur = 255;
    int *socketClient = (int*)malloc(maxJoueur*sizeof(int));

    // TEST BDD
    MYSQL *con = connectBdd();

    if (con == NULL) {
        exit(1);
    }

    // FIXME to do statistics

//    if (createUser(con, "tibo", "mdpdeouf") == 0) {
//        printf("L'utilisateur a bien été créer\n");
//    } else {
//        printf("L'utilisateur n'a pas pu être créer\n");
//    }

//    if(connectUser(con, "tibo", "mdpdeouf") == 1) {
//        printf("L'utilisateur existe et le mot de passe sont correct\n");
//
//        addStats(con, getUsernameConnected(), 2);
//    } else {
//        printf("L'utilisateur n'existe pas ou le mot de passe est incorrect\n");
//    }

//    Stats test = getStats(con, "tibo");
//    printf("STATS : %d\n", test.nbDrawConnect4);
//
//    addStats(con,"tibo",1);

//    closeBdd(con);
//
//    int wait;
//    scanf("%d", &wait);

    // FIN TEST BDD

    args.socketPlayer1 = 0;
    args.socketPlayer2 = 0;
    args.GameId = 0;

    while (nbJoueur < maxJoueur){
        listen(socketServer, maxJoueur - nbJoueur);
        addrClient_lenght = sizeof(addrClient);
        socketClient[nbJoueur] = accept(socketServer, (struct sockaddr *) &addrClient, &addrClient_lenght);

        if (socketClient[nbJoueur] < 0) {
            printf("ERREUR DE CONNEXION AVEC LE CLIENT !\n");
        }
        printf("CONNEXION ACCEPTER DU CLIENT NUMERO :  %d\n", nbJoueur);

        char respons[6];
        recv(socketClient[nbJoueur], respons, 5, 0);
        respons[5] = '\0';
        printf("RECU1 : %s\n", respons);

        LoginArgs * args2 = malloc(sizeof(LoginArgs));
        if (args2 == NULL) {
            printf("ERROR ALLOCATING LOGINARGS !\n");
            exit(EXIT_FAILURE);
        }
        args2->socketClient = socketClient[nbJoueur];
        args2->nbJoueur = nbJoueur;

        pthread_t thread_id;
        if (strcmp(respons, "LOGIN") == 0) {
            pthread_create(&thread_id, NULL, (void*)login, args2);
        }
        nbJoueur++;
    }

    // TODO FREE LOGINARGS

    // Close Socket client
    for (int i = 0; i < nbJoueur; ++i) {
        close(socketClient[i]);
    }
    // Close Socket server
    close(socketServer);

    return 0;
}
