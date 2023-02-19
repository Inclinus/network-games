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

// action = 1 -> login
// action = 2 -> register
int login(int * socketClient, int action){
    if (action == 1) {
        send(*socketClient, "LOGINCLI", 8, 0);
    } else if (action == 2) {
        send(*socketClient, "REGISTER", 8, 0);
    }
    char buffer[25];
    printf("Quel est votre pseudo : \n");
    scanf("%s", buffer);
    send(*socketClient, buffer, sizeof(buffer), 0);
    printf("Quel est votre mot de passe : \n");
    scanf("%s", buffer);
    send(*socketClient, buffer, sizeof(buffer), 0);
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

void displayConfig(SDL_Renderer * renderer, char * text){
    changeColor(renderer,255,255,255);
    createFilledRectangle(0,0,500,400,renderer);
    changeColor(renderer,0,0,0);
    createFilledRectangle(100,150,300,50,renderer);

    if(strlen(text)>0){
        createTextZone(renderer,text,105,160,255,255,255);
    }
    updateRenderer(renderer);
}

void str_to_uint16(const char *str, uint16_t *res) {
    char *end;
    long val = strtol(str, &end, 10);
    *res = (uint16_t)val;
}

void configServer(){
    initSDL();

    char * serverIp = NULL;
    uint16_t serverPort = 0;
    char * username = NULL;
    char * password = NULL;

    FILE* file;

    file = fopen("client/settings.txt", "r");
    if (file == NULL) {
        printf("Failed to open config file.\n ERRNO = %d\n",errno);
        exit(1);
    }

    char line[100];

    while (fgets(line, 10, file)) {
        char* token;
        char* key;
        char* value;
        if((token = strtok(line, "="))==NULL){
            continue;
        }
        key = token;
        if((token = strtok(NULL, "\n"))==NULL){
            continue;
        }
        value = token;

        if (strcmp(key, "server_ip") == 0) {
            printf("Server IP: %s\n", value);
            if((serverIp=malloc(sizeof(char)* strlen(value)))!=NULL){
                strcpy(serverIp,value);
            } else {
                SDL_ExitWithError("ERROR ALLOCATING SERVERIP");
            }
        } else if (strcmp(key, "server_port") == 0) {
            str_to_uint16(value,&serverPort);
            printf("Server Port: %d\n", serverPort);
        } else if (strcmp(key, "username") == 0) {
            printf("User: %s\n", value);
            if((username=malloc(sizeof(char)* strlen(value)))!=NULL){
                strcpy(username,value);
            } else {
                SDL_ExitWithError("ERROR ALLOCATING USERNAME");
            }
        } else if (strcmp(key, "password") == 0) {
            printf("Password: %s\n", value);
            if((password=malloc(sizeof(char)* strlen(value)))!=NULL){
                strcpy(password,value);
            } else {
                SDL_ExitWithError("ERROR ALLOCATING PASSWORD");
            }
        }
    }

    fclose(file);

    SDL_Window * configWindow = SDL_CreateWindow("PARAMETRES",50,50,500,400,0);
    SDL_Renderer * configRenderer = SDL_CreateRenderer(configWindow, -1, 0);

    char * text = NULL;
    if(serverIp!=NULL){
        printf("%s",serverIp);
        text = serverIp;
    } else {
        text =  malloc(sizeof(char)*2);
        text[0] = ' ';
        text[1] = '\0';
    }

    displayConfig(configRenderer,text);

    int quit = 0;
    while(!quit){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch (event.type) {
                case SDL_QUIT:
                    quit=1;
                    break;
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_RETURN){
                        quit=2;
                        break;
                    } else if(event.key.keysym.sym == SDLK_BACKSPACE){
                        int length = strlen(text);
                        if(length>0){
                            text = realloc(text,length*sizeof(char));
                            text[length-1] = '\0';
                        }
                    } else if(event.key.keysym.sym >= 97 && event.key.keysym.sym <= 122 || event.key.keysym.sym >= 48 && event.key.keysym.sym <= 57){
                        int length = strlen(text);
                        text = realloc(text,length* sizeof(char)+2);
                        const char * keyname = SDL_GetKeyName(event.key.keysym.sym);
                        char keyChar = tolower(keyname[0]);

                        text[length] = keyChar;
                        text[length+1] = '\0';
                    } else if(event.key.keysym.sym >= 1073741913 && event.key.keysym.sym <= 1073741923){
                        int length = strlen(text);
                        text = realloc(text,length* sizeof(char)+2);
                        const char * keyname = SDL_GetKeyName(event.key.keysym.sym);
                        char keyChar = keyname[7];

                        text[length] = keyChar;
                        text[length+1] = '\0';
                    }
            }
            displayConfig(configRenderer,text);
        }
    }


}


int main() {
    eventManagerInit();
    int socketClient = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrServer;
    // gethostbyname() -> permet d'utilise le DNS
    // projetc.neo-serv.fr -> 92.222.131.57

    struct hostent *ipserveur;
    //ipserveur = gethostbyname("localhost");
    ipserveur = gethostbyname("projetc.neo-serv.fr");

    if (ipserveur == NULL) {
        printf("ERREUR, l'host n'a pas été trouver\n");
        exit(0);
    }

    //addrServer.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrServer.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)*ipserveur->h_addr_list));
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(4444);

    if (connect(socketClient, (const struct sockaddr *) &addrServer, sizeof(addrServer)) < 0) {
        perror("ERREUR DE CONNEXION");
        exit(1);
    }
    printf("[DEBUG] CONNECTER !\n");
    clientSocket = &socketClient;

    printf("En attente d'adversaire ...\n");

//    tictactoe(clientSocket);
//
//    send(socketClient, "LOGIN", 5, 0);
//
//    int action_login;
//    printf("1 - Se connecter\n 2 - S'inscrire\n");
//    scanf("%d", &action_login);
//    if (action_login == 1) {
//        while (login(&socketClient, 1) == 0);
//    } else if (action_login == 2) {
//        login(&socketClient, 2);
//    } else {
//        printf("ERREUR DE CHOIX !\n");
//        exit(1);
//    }
    // Clear event queues to be sure no events is stucked
    clearQueues();

    printf("[DEBUG] FIN DE L'AUTHENTIFICATION !\n");

    initSDLGUIs(&clientRunning,&socketClient);

    while (clientRunning) {
        loadMainMenu();
    }

    close(socketClient);

    return 0;
}
