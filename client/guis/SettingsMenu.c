#include <errno.h>
#include "SettingsMenu.h"
#include "MainMenu.h"

typedef enum {
    SERVER_IP = 0,
    SERVER_PORT = 1,
    USERNAME = 2,
    PASSWORD = 3
} Input;

typedef struct {
    char * server_ip;
    uint16_t server_port;
    char * username;
    char * password;
} Configuration;

void displayConfig(SDL_Renderer * renderer, char * ipText, char * portText, char * username, char * password, Input selectedInput);
void str_to_uint16(const char *str, uint16_t *res);
void initInputButtons();
void saveAll(char * ip, char * port, char * username, char * password);

SDL_bool * settingsMenuRunning = NULL;

Button * ipInput;

Button * portInput;
Button * usernameInputSettings;
Button * passwordInputSettings;

Button * goBack;

void settingsMenu(SDL_Renderer * rendererMenu){
    settingsMenuRunning = malloc(sizeof(SDL_bool));
    if(settingsMenuRunning==NULL){
        SDL_ExitWithError("ERROR ALLOCATING SETTINGSMENURUNNING SDLBOOL");
    }
    *settingsMenuRunning = SDL_TRUE;

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

    char * usernameText = NULL;
    if(config.username!=NULL){
        printf("%s",config.username);
        usernameText = malloc(sizeof(char)* strlen(config.username)+1);
        strcpy(usernameText,config.username);
    } else {
        usernameText =  malloc(sizeof(char));
        usernameText[0] = '\0';
    }

    char * passwordText = NULL;
    if(config.password!=NULL){
        printf("%s",config.password);
        passwordText = malloc(sizeof(char)* strlen(config.password)+1);
        strcpy(passwordText,config.password);
    } else {
        passwordText =  malloc(sizeof(char));
        passwordText[0] = '\0';
    }

    char * selectedInputText = serverIpText;
    Input selectedInput = SERVER_IP;
    displayConfig(rendererMenu, serverIpText,serverPortText,usernameText,passwordText,selectedInput);

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
                    }  else if(x>usernameInputSettings->beginX && usernameInputSettings->endX>x && y>usernameInputSettings->beginY && usernameInputSettings->endY>y){
                        selectedInput = USERNAME;
                        selectedInputText = usernameText;
                    }  else if(x>passwordInputSettings->beginX && passwordInputSettings->endX>x && y>passwordInputSettings->beginY && passwordInputSettings->endY>y){
                        selectedInput = PASSWORD;
                        selectedInputText = passwordText;
                    } else if(x>goBack->beginX && goBack->endX>x && y>goBack->beginY && goBack->endY>y){
                        *settingsMenuRunning = SDL_FALSE;
                        loadMainMenu();
                        break;
                    }
                    displayConfig(rendererMenu, serverIpText,serverPortText,usernameText,passwordText,selectedInput);
                    break;
                case SDL_QUIT:
                    *settingsMenuRunning = SDL_FALSE;
                    loadMainMenu();
                    break;
                case SDL_KEYDOWN:;
                    int length = strlen(selectedInputText);
                    if(event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_RETURN){
                        *settingsMenuRunning = SDL_FALSE;
                        saveAll(serverIpText,serverPortText,usernameText,passwordText);
                        break;
                    } else if(event.key.keysym.sym == SDLK_BACKSPACE){
                        if(length>0){
                            selectedInputText = realloc(selectedInputText, length * sizeof(char));
                            if(selectedInputText==NULL){
                                SDL_ExitWithError("ERROR REALLOCATING SELECTEDINPUTTEXT");
                            }
                            selectedInputText[length - 1] = '\0';
                        }
                    } else if(selectedInput==SERVER_IP && length==21 || selectedInput==SERVER_PORT && length==5 || selectedInput==USERNAME && length==21 || selectedInput==PASSWORD && length==21){
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
                    } else if(event.key.keysym.sym >= 1073741913 && event.key.keysym.sym <= 1073741923 || event.key.keysym.sym == 1073741910){
                        selectedInputText = realloc(selectedInputText, length * sizeof(char) + 2);
                        if(selectedInputText==NULL){
                            SDL_ExitWithError("ERROR REALLOCATING SELECTEDINPUTTEXT");
                        }
                        const char * keyname = SDL_GetKeyName(event.key.keysym.sym);
                        char keyChar = keyname[7];

                        selectedInputText[length] = keyChar;
                        selectedInputText[length + 1] = '\0';
                    }
                    displayConfig(rendererMenu, serverIpText,serverPortText,usernameText,passwordText,selectedInput);
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


void displayConfig(SDL_Renderer * renderer, char * ipText, char * portText, char * userText, char * passwordText, Input selectedInput){
    SDL_RenderClear(renderer);
//    changeColor(renderer,255,255,255);
//    createFilledRectangle(0,0,720,480,renderer);
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
    SDL_Log("STRLEN DE USERNAMETEXT = %d",strlen(userText));
    if(selectedInput == USERNAME) {
        if(strlen(userText)>=1 && strlen(userText)<21) {
            createButtonColor(renderer, *usernameInputSettings, userText, 0, 255, 0);
        } else {
            createButtonColor(renderer, *usernameInputSettings, "INSERT USERNAME HERE", 0, 255, 0);
        }
    } else {
        if(strlen(userText)>=1 && strlen(userText)<21) {
            createButtonColor(renderer, *usernameInputSettings, userText, 0, 0, 0);
        } else {
            createButtonColor(renderer, *usernameInputSettings, "INSERT USERNAME HERE", 0, 0, 0);
        }
    }
    SDL_Log("STRLEN DE PASSWORDTEXT = %d",strlen(passwordText));
    if(selectedInput == PASSWORD) {
        if(strlen(passwordText)>=1 && strlen(passwordText)<21) {
            createButtonColor(renderer, *passwordInputSettings, passwordText, 0, 255, 0);
        } else {
            createButtonColor(renderer, *passwordInputSettings, "INSERT PASSWORD HERE", 0, 255, 0);
        }
    } else {
        if(strlen(passwordText)>=1 && strlen(passwordText)<21) {
            createButtonColor(renderer, *passwordInputSettings, passwordText, 0, 0, 0);
        } else {
            createButtonColor(renderer, *passwordInputSettings, "INSERT PASSWORD HERE", 0, 0, 0);
        }
    }

    createButton(renderer,*goBack, "retour");

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
    ipInput->beginX = 220;
    ipInput->beginY = 80;
    ipInput->endX = 520;
    ipInput->endY = 130;

    portInput = malloc(sizeof(Button));
    if(portInput==NULL){
        SDL_ExitWithError("ERROR ALLOCATING PORT INPUT BUTTON");
    }
    portInput->beginX = 220;
    portInput->beginY = 150;
    portInput->endX = 520;
    portInput->endY = 200;

    usernameInputSettings = malloc(sizeof(Button));
    if(usernameInputSettings==NULL){
        SDL_ExitWithError("ERROR ALLOCATING USERNAME INPUT BUTTON");
    }
    usernameInputSettings->beginX = 220;
    usernameInputSettings->beginY = 220;
    usernameInputSettings->endX = 520;
    usernameInputSettings->endY = 270;

    passwordInputSettings = malloc(sizeof(Button));
    if(passwordInputSettings==NULL){
        SDL_ExitWithError("ERROR ALLOCATING PASSWORD INPUT BUTTON");
    }
    passwordInputSettings->beginX = 220;
    passwordInputSettings->beginY = 290;
    passwordInputSettings->endX = 520;
    passwordInputSettings->endY = 340;

    goBack = malloc(sizeof(Button));
    if(goBack==NULL){
        SDL_ExitWithError("ERROR ALLOCATING GO BACK BUTTON");
    }
    goBack->beginX = 20;
    goBack->beginY = 430;
    goBack->endX = 150;
    goBack->endY = 470;
}