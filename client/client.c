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

typedef enum {
    SERVER_IP = 0,
    SERVER_PORT = 1
} Input;

typedef struct {
    char * server_ip;
    uint16_t server_port;
    char * username;
    char * password;
} Configuration;

Button * ipInput;

Button * portInput;

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

void displayConfig(SDL_Renderer * renderer, char * ipText, char * portText, Input selectedInput){

    changeColor(renderer,255,255,255);
    createFilledRectangle(0,0,500,400,renderer);
    SDL_Log("STRLEN DE IPTEXT = %d",strlen(ipText));
    if(selectedInput == SERVER_IP) {
        if(strlen(ipText)>=1 && strlen(ipText)<22) {
            createButtonColor(renderer, *ipInput, ipText, 0, 255, 0);
        } else {
            createButtonColor(renderer, *ipInput, "INSERT IP HERE", 0, 255, 0);
        }
    } else {
        if(strlen(ipText)>=1 && strlen(ipText)<22) {
            createButtonColor(renderer, *ipInput, ipText, 0, 0, 0);
        } else {
            createButtonColor(renderer, *ipInput, "INSERT IP HERE", 0, 0, 0);
        }
    }
    SDL_Log("STRLEN DE PORTTEXT = %d",strlen(portText));
    if(selectedInput == SERVER_PORT) {
        if(strlen(portText)>=1 && strlen(portText)<6) {
            createButtonColor(renderer, *portInput, portText, 0, 255, 0);
        } else {
            createButtonColor(renderer, *portInput, "INSERT PORT HERE", 0, 255, 0);
        }
    } else {
        if(strlen(portText)>=1 && strlen(portText)<6) {
            createButtonColor(renderer, *portInput, portText, 0, 0, 0);
        } else {
            createButtonColor(renderer, *portInput, "INSERT PORT HERE", 0, 0, 0);
        }
    }

    updateRenderer(renderer);
}

void str_to_uint16(const char *str, uint16_t *res) {
    char *end;
    long val = strtol(str, &end, 10);
    *res = (uint16_t)val;
}

void initInputButtons(){
    ipInput = malloc(sizeof(Button));
    if(ipInput==NULL){
        SDL_ExitWithError("ERROR ALLOCATING IP INPUT BUTTON");
    }
    ipInput->beginX = 100;
    ipInput->beginY = 90;
    ipInput->endX = 400;
    ipInput->endY = 140;

    portInput = malloc(sizeof(Button));
    if(portInput==NULL){
        SDL_ExitWithError("ERROR ALLOCATING PORT INPUT BUTTON");
    }
    portInput->beginX = 100;
    portInput->beginY = 250;
    portInput->endX = 400;
    portInput->endY = 300;
}



void configServer(){
    initSDL();
    initInputButtons();

    FILE* file;

    file = fopen("client/settings.txt", "r");
    if (file == NULL) {
        printf("Failed to open config file.\n ERRNO = %d\n",errno);
        exit(1);
    }

    char line[1024];
    char field_name[124], field_value[124];
    Configuration config = {"", 0, "", ""};

    while (fgets(line, 1024, file) != NULL) {
        sscanf(line, "%[^=]=%[^\n]", field_name, field_value);
        if (strcmp(field_name, "server_ip") == 0) {
            config.server_ip = malloc(sizeof(char)* strlen(field_value)+1);
            printf("Server IP: %s\n", field_value);
            strcpy(config.server_ip, field_value);
        } else if (strcmp(field_name, "server_port") == 0) {
            str_to_uint16(field_value,&config.server_port);
            printf("Server Port: %d\n", config.server_port);
        } else if (strcmp(field_name, "username") == 0) {
            config.username = malloc(sizeof(char)* strlen(field_value)+1);
            printf("User: %s\n", field_value);
            strcpy(config.username, field_value);
        } else if (strcmp(field_name, "password") == 0) {
            config.password = malloc(sizeof(char)* strlen(field_value)+1);
            printf("Password: %s\n", field_value);
            strcpy(config.password, field_value);
        } else {
            printf("Unknown field: %s\n", field_name);
        }
    }

    fclose(file);
    SDL_Window * configWindow = SDL_CreateWindow("PARAMETRES",50,50,500,400,0);
    SDL_Renderer * configRenderer = SDL_CreateRenderer(configWindow, -1, 0);

    char * serverIpText = NULL;
    if(config.server_ip!=NULL){
        printf("%s",config.server_ip);
        serverIpText = malloc(sizeof(char)* strlen(config.server_ip)+1);
        strcpy(serverIpText,config.server_ip);
    } else {
        serverIpText =  malloc(sizeof(char));
        serverIpText[0] = '\0';
    }

    char * serverPortText = NULL;
    if(config.server_port!=0){
        printf("%d",config.server_port);
        serverPortText = malloc(sizeof(char)*5);
        memset(serverPortText, '\0', 5);
        sprintf(serverPortText, "%"PRIu16"", config.server_port);
    } else {
        serverPortText =  malloc(sizeof(char));
        serverPortText[0] = '\0';
    }

    char * selectedInputText = serverIpText;
    Input selectedInput = SERVER_IP;
    displayConfig(configRenderer, serverIpText,serverPortText,selectedInput);

    int quit = 0;
    while(!quit){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch (event.type) {
                case SDL_MOUSEBUTTONDOWN:;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>portInput->beginX && portInput->endX>x && y>portInput->beginY && portInput->endY>y){
                        selectedInput = SERVER_PORT;
                        selectedInputText = serverPortText;
                    } else if(x>ipInput->beginX && ipInput->endX>x && y>ipInput->beginY && ipInput->endY>y){
                        selectedInput = SERVER_IP;
                        selectedInputText = serverIpText;
                    }
                    displayConfig(configRenderer, serverIpText,serverPortText,selectedInput);
                    break;
                case SDL_QUIT:
                    quit=1;
                    break;
                case SDL_KEYDOWN:;
                    int length = strlen(selectedInputText);
                    if(event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_RETURN){
                        quit=2;
                        break;
                    } else if(event.key.keysym.sym == SDLK_BACKSPACE){
                        if(length>0){
                            selectedInputText = realloc(selectedInputText, length * sizeof(char));
                            if(selectedInputText==NULL){
                                SDL_ExitWithError("ERROR REALLOCATING SELECTEDINPUTTEXT");
                            }
                            selectedInputText[length - 1] = '\0';
                        }
                    } else if(selectedInput==SERVER_IP && length==21 || selectedInput==SERVER_PORT && length==5){
                        break;
                    } else if(event.key.keysym.sym == 59){
                        selectedInputText = realloc(selectedInputText, length * sizeof(char) + 2);
                        if(selectedInputText==NULL){
                            SDL_ExitWithError("ERROR REALLOCATING SELECTEDINPUTTEXT");
                        }
                        selectedInputText[length] = '.';
                        selectedInputText[length + 1] = '\0';
                    } else if(event.key.keysym.sym >= 97 && event.key.keysym.sym <= 122 || event.key.keysym.sym >= 48 && event.key.keysym.sym <= 57){
                        selectedInputText = realloc(selectedInputText, length * sizeof(char) + 2);
                        if(selectedInputText==NULL){
                            SDL_ExitWithError("ERROR REALLOCATING SELECTEDINPUTTEXT");
                        }
                        const char * keyname = SDL_GetKeyName(event.key.keysym.sym);
                        char keyChar = tolower(keyname[0]);

                        selectedInputText[length] = keyChar;
                        selectedInputText[length + 1] = '\0';
                    } else if(event.key.keysym.sym >= 1073741913 && event.key.keysym.sym <= 1073741923){
                        selectedInputText = realloc(selectedInputText, length * sizeof(char) + 2);
                        if(selectedInputText==NULL){
                            SDL_ExitWithError("ERROR REALLOCATING SELECTEDINPUTTEXT");
                        }
                        const char * keyname = SDL_GetKeyName(event.key.keysym.sym);
                        char keyChar = keyname[7];

                        selectedInputText[length] = keyChar;
                        selectedInputText[length + 1] = '\0';
                    }
                    displayConfig(configRenderer, serverIpText,serverPortText,selectedInput);
                    break;
            }
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
//    addrServer.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)*ipserveur->h_addr_list));
//    addrServer.sin_family = AF_INET;
//    addrServer.sin_port = htons(4444);
//
//    if (connect(socketClient, (const struct sockaddr *) &addrServer, sizeof(addrServer)) < 0) {
//        perror("ERREUR DE CONNEXION");
//        exit(1);
//    }
//    printf("[DEBUG] CONNECTER !\n");
//    clientSocket = &socketClient;
//
//    printf("En attente d'adversaire ...\n");

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

    configServer();

//    initSDLGUIs(&clientRunning,&socketClient);
//
//    while (clientRunning) {
//        loadMainMenu();
//    }

    close(socketClient);

    return 0;
}
