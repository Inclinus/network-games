#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include "tictactoe/tictactoe.h"
#include "../events/EventManager.h"
#include "connect4/connect4.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include "../sdl-utils/SDLUtils.h"
#include "guis/MainMenu.h"
#include <SDL2/SDL_ttf.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <errno.h>
#include "../sdl-utils/SDLUtils.h"

SDL_bool clientRunning = SDL_TRUE;
int * clientSocket;


char * getUsername(){
    FILE* file;

    file = fopen("client/settings.txt", "r");
    if (file == NULL) {
        printf("Failed to open config file.\n ERRNO = %d\n",errno);
        exit(1);
    }

    char line[1024];
    char field_name[124], field_value[124];

    char * username = NULL;

    while (fgets(line, 1024, file) != NULL) {
        sscanf(line, "%[^=]=%[^\n]", field_name, field_value);
        if (strcmp(field_name, "username") == 0) {
            username = malloc(sizeof(char)* strlen(field_value)+1);
            printf("Username: %s\n", field_value);
            strcpy(username, field_value);
        }
    }

    fclose(file);

    return username;
}

char * getPassword(){
    FILE* file;

    file = fopen("client/settings.txt", "r");
    if (file == NULL) {
        printf("Failed to open config file.\n ERRNO = %d\n",errno);
        exit(1);
    }

    char line[1024];
    char field_name[124], field_value[124];

    char * password = NULL;

    while (fgets(line, 1024, file) != NULL) {
        sscanf(line, "%[^=]=%[^\n]", field_name, field_value);
        if (strcmp(field_name, "password") == 0) {
            password = malloc(sizeof(char)* strlen(field_value)+1);
            printf("Password: %s\n", field_value);
            strcpy(password, field_value);
        }
    }

    fclose(file);

    return password;
}

// action = 1 -> login
// action = 2 -> register
int login(int * socketClient, int action){
    char username[25];
    char password[25];
    if (action == 1) {
        send(*socketClient, "LOGINCLI", 8, 0);
        usleep(100000);
        send(*socketClient, getUsername(), 25, 0);
        usleep(100000);
        send(*socketClient, getPassword(), 25, 0);
    } else if (action == 2) {
        send(*socketClient, "REGISTER", 8, 0);

        printf("Quel est votre pseudo : \n");
        scanf("%s", username);
        send(*socketClient, username, sizeof(username), 0);

        printf("Quel est votre mot de passe : \n");
        scanf("%s", password);
        send(*socketClient, password, sizeof(password), 0);
    }

    char result[3];
    recv(*socketClient, result, sizeof(result), 0);
    result[2] = '\0';
    printf("RECU : %s\n", result);
    if(strcmp(result, "OK") == 0){
        printf("OK !\n");

        return 1;
    }else{
        printf("NOK !\n");
        return 0;
    }
}


char * getServerIp(){
    FILE* file;

    file = fopen("client/settings.txt", "r");
    if (file == NULL) {
        printf("Failed to open config file.\n ERRNO = %d\n",errno);
        exit(1);
    }

    char line[1024];
    char field_name[124], field_value[124];

    char * serverIp = NULL;

    while (fgets(line, 1024, file) != NULL) {
        sscanf(line, "%[^=]=%[^\n]", field_name, field_value);
        if (strcmp(field_name, "server_ip") == 0) {
            serverIp = malloc(sizeof(char)* strlen(field_value)+1);
            printf("Server IP: %s\n", field_value);
            strcpy(serverIp, field_value);
        }
    }

    fclose(file);

    return serverIp;
}

void client_str_to_uint16(const char *str, uint16_t *res) {
    char *end;
    long val = strtol(str, &end, 10);
    *res = (uint16_t)val;
}

uint16_t getServerPort(){
    FILE* file;

    file = fopen("client/settings.txt", "r");
    if (file == NULL) {
        printf("Failed to open config file.\n ERRNO = %d\n",errno);
        exit(1);
    }

    char line[1024];
    char field_name[124], field_value[124];

    uint16_t result = 0;

    while (fgets(line, 1024, file) != NULL) {
        sscanf(line, "%[^=]=%[^\n]", field_name, field_value);
        if (strcmp(field_name, "server_port") == 0) {
            printf("Server PORT: %s\n", field_value);
            client_str_to_uint16(field_value,&result);
        }
    }

    fclose(file);

    return result;
}

int main() {
    eventManagerInit();
    int socketClient = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrServer;
    // gethostbyname() -> permet d'utilise le DNS
    // projetc.neo-serv.fr -> 92.222.131.57

    struct hostent *ipserveur;
    //ipserveur = gethostbyname("localhost");
    //ipserveur = gethostbyname("projetc.neo-serv.fr");
    char *serverIp = getServerIp();
    ipserveur = gethostbyname(serverIp);

    if (ipserveur == NULL) {
        printf("ERREUR, l'host n'a pas été trouver\n");
        exit(0);
    }

    //addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrServer.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)*ipserveur->h_addr_list));
    addrServer.sin_family = AF_INET;
    //addrServer.sin_port = htons(4444);
    addrServer.sin_port = htons(getServerPort());
    
    if (connect(socketClient, (const struct sockaddr *) &addrServer, sizeof(addrServer)) < 0) {
        perror("ERREUR DE CONNEXION");
        exit(1);
    }
    printf("[DEBUG] CONNECTER !\n");
    clientSocket = &socketClient;

    printf("En attente d'adversaire ...\n");

    send(socketClient, "LOGIN", 5, 0);

    int action_login;
    printf("1 - Se connecter\n 2 - S'inscrire\n");
    scanf("%d", &action_login);
    if (action_login == 1) {
        while (login(&socketClient, 1) == 0);
    } else if (action_login == 2) {
        login(&socketClient, 2);
    } else {
        printf("ERREUR DE CHOIX !\n");
        exit(1);
    }
    // Clear event queues to be sure no events is stucked
    clearQueues();

    printf("[DEBUG] FIN DE L'AUTHENTIFICATION !\n");

    // FIXME add the config GUI
    //configServer();

    initSDLGUIs(&clientRunning,&socketClient);

    while (clientRunning) {
        loadMainMenu();
    }

    close(socketClient);

    return 0;
}
