#include <errno.h>
#include "SettingsMenu.h"
#include "MainMenu.h"

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

void displayConfig(SDL_Renderer * renderer, char * ipText, char * portText, Input selectedInput);
void str_to_uint16(const char *str, uint16_t *res);
void initInputButtons();
void saveAll(char * ip, char * port, char * username, char * password);

SDL_bool * settingsMenuRunning = NULL;

Button * ipInput;

Button * portInput;

void settingsMenu(SDL_Renderer * rendererMenu){
    settingsMenuRunning = malloc(sizeof(SDL_bool));
    if(settingsMenuRunning==NULL){
        SDL_ExitWithError("ERROR ALLOCATING SETTINGSMENURUNNING SDLBOOL");
    }
    *settingsMenuRunning = SDL_TRUE;

    Button goBack;
    goBack.beginX = 20;
    goBack.beginY = 430;
    goBack.endX = 150;
    goBack.endY = 470;

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
    displayConfig(rendererMenu, serverIpText,serverPortText,selectedInput);

    while(*settingsMenuRunning){
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
                    } else if(x>goBack.beginX && goBack.endX>x && y>goBack.beginY && goBack.endY>y){
                        *settingsMenuRunning = SDL_FALSE;
                        loadMainMenu();
                        break;
                    }
                    displayConfig(rendererMenu, serverIpText,serverPortText,selectedInput);
                    break;
                case SDL_QUIT:
                    *settingsMenuRunning = SDL_FALSE;
                    loadMainMenu();
                    break;
                case SDL_KEYDOWN:;
                    int length = strlen(selectedInputText);
                    if(event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_RETURN){
                        *settingsMenuRunning = SDL_FALSE;
                        saveAll(serverIpText,serverPortText,config.username,config.password);
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
                    displayConfig(rendererMenu, serverIpText,serverPortText,selectedInput);
                    break;
            }
        }
    }
}

void saveAll(char * ip, char * port, char * username, char * password){
    FILE* file = fopen("client/settings.txt", "w");
    if (file == NULL) {
        printf("Failed to open config file.\n ERRNO = %d\n",errno);
        exit(1);
    }

    // Write the properties to the file
    fprintf(file, "server_ip=%s\n", ip);
    fprintf(file, "server_port=%s\n", port);
    fprintf(file, "username=%s\n", username);
    fprintf(file, "password=%s\n", password);

    // Close the file
    fclose(file);
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