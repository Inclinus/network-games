#include "AuthenticationMenu.h"

#include <errno.h>
#include "SettingsMenu.h"
#include "MainMenu.h"

typedef enum {
    USERNAME = 0,
    PASSWORD = 1
} Input;

typedef struct {
    char * server_ip;
    char * server_port;
    char * username;
    char * password;
} ConfigurationAuth;

void authenticationDisplayConfig(SDL_Renderer * renderer, char * usernameText, char * passwordText, Input selectedInput);
void auth_str_to_uint16(const char *str, uint16_t *res);
void initAuthenticationInputButtons();
void authenticationSaveAll(char * ip, char * port, char * username, char * password);

SDL_bool * authMenuRunning = NULL;

Button * usernameInput;

Button * passwordInput;

void authenticationMenu(SDL_Renderer * rendererMenu){
    authMenuRunning = malloc(sizeof(SDL_bool));
    if(authMenuRunning==NULL){
        SDL_ExitWithError("ERROR ALLOCATING SETTINGSMENURUNNING SDLBOOL");
    }
    *authMenuRunning = SDL_TRUE;

    Button goBack;
    goBack.beginX = 20;
    goBack.beginY = 430;
    goBack.endX = 150;
    goBack.endY = 470;

    initAuthenticationInputButtons();

    FILE* file;

    file = fopen("client/settings.txt", "r");
    if (file == NULL) {
        printf("Failed to open config file.\n ERRNO = %d\n",errno);
        exit(1);
    }

    char line[1024];
    char field_name[124], field_value[124];
    ConfigurationAuth config = {"", "", "", ""};

    while (fgets(line, 1024, file) != NULL) {
        sscanf(line, "%[^=]=%[^\n]", field_name, field_value);
        if (strcmp(field_name, "server_ip") == 0) {
            config.server_ip = malloc(sizeof(char)* strlen(field_value)+1);
            printf("Server IP: %s\n", field_value);
            strcpy(config.server_ip, field_value);
        } else if (strcmp(field_name, "server_port") == 0) {
            config.server_port = malloc(sizeof(char)* strlen(field_value)+1);
            printf("Server port: %s\n", field_value);
            strcpy(config.server_port, field_value);
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

    char * usernameText = NULL;
    if(config.username!=NULL){
        printf("%s",config.username);
        usernameText = malloc(sizeof(char) * strlen(config.username) + 1);
        strcpy(usernameText, config.username);
    } else {
        usernameText =  malloc(sizeof(char));
        usernameText[0] = '\0';
    }

    char * passwordText = NULL;
    if(config.password!=0){
        printf("%s",config.password);
        usernameText = malloc(sizeof(char) * strlen(config.password) + 1);
        strcpy(usernameText, config.password);
    } else {
        passwordText =  malloc(sizeof(char));
        passwordText[0] = '\0';
    }

    char * selectedInputText = usernameText;
    Input selectedInput = USERNAME;
    authenticationDisplayConfig(rendererMenu, usernameText, passwordText, selectedInput);

    while(*authMenuRunning){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch (event.type) {
                case SDL_MOUSEBUTTONDOWN:;
                    int x = event.button.x;
                    int y = event.button.y;
                    if(x>passwordInput->beginX && passwordInput->endX>x && y>passwordInput->beginY && passwordInput->endY>y){
                        selectedInput = PASSWORD;
                        selectedInputText = passwordText;
                    } else if(x>usernameInput->beginX && usernameInput->endX>x && y>usernameInput->beginY && usernameInput->endY>y){
                        selectedInput = USERNAME;
                        selectedInputText = usernameText;
                    } else if(x>goBack.beginX && goBack.endX>x && y>goBack.beginY && goBack.endY>y){
                        *authMenuRunning = SDL_FALSE;
                        loadMainMenu();
                        break;
                    }
                    authenticationDisplayConfig(rendererMenu, usernameText, passwordText, selectedInput);
                    break;
                case SDL_QUIT:
                    *authMenuRunning = SDL_FALSE;
                    loadMainMenu();
                    break;
                case SDL_KEYDOWN:;
                    int length = strlen(selectedInputText);
                    if(event.key.keysym.sym == SDLK_KP_ENTER || event.key.keysym.sym == SDLK_RETURN){
                        *authMenuRunning = SDL_FALSE;
                        authenticationSaveAll(config.server_ip,config.server_port, usernameText, passwordText);
                        break;
                    } else if(event.key.keysym.sym == SDLK_BACKSPACE){
                        if(length>0){
                            selectedInputText = realloc(selectedInputText, length * sizeof(char));
                            if(selectedInputText==NULL){
                                SDL_ExitWithError("ERROR REALLOCATING SELECTEDINPUTTEXT");
                            }
                            selectedInputText[length - 1] = '\0';
                        }
                    } else if(selectedInput==USERNAME && length==21 || selectedInput==PASSWORD && length==5){
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
                    authenticationDisplayConfig(rendererMenu, usernameText, passwordText, selectedInput);
                    break;
            }
        }
    }
}

void authenticationSaveAll(char * ip, char * port, char * username, char * password){
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


void authenticationDisplayConfig(SDL_Renderer * renderer, char * usernameText, char * passwordText, Input selectedInput){

    changeColor(renderer,255,255,255);
    createFilledRectangle(0,0,500,400,renderer);
    SDL_Log("STRLEN DE USERNAMETEXT = %d",strlen(usernameText));
    if(selectedInput == USERNAME) {
        if(strlen(usernameText) >= 1 && strlen(usernameText) < 22) {
            createButtonColor(renderer, *usernameInput, usernameText, 0, 255, 0);
        } else {
            createButtonColor(renderer, *usernameInput, "INSERT USERNAME HERE", 0, 255, 0);
        }
    } else {
        if(strlen(usernameText) >= 1 && strlen(usernameText) < 22) {
            createButtonColor(renderer, *usernameInput, usernameText, 0, 0, 0);
        } else {
            createButtonColor(renderer, *usernameInput, "INSERT USERNAME HERE", 0, 0, 0);
        }
    }
    SDL_Log("STRLEN DE PASSWORDTEXT = %d",strlen(passwordText));
    if(selectedInput == PASSWORD) {
        if(strlen(passwordText) >= 1 && strlen(passwordText) < 22) {
            createButtonColor(renderer, *passwordInput, passwordText, 0, 255, 0);
        } else {
            createButtonColor(renderer, *passwordInput, "INSERT PASSWORD HERE", 0, 255, 0);
        }
    } else {
        if(strlen(passwordText) >= 1 && strlen(passwordText) < 22) {
            createButtonColor(renderer, *passwordInput, passwordText, 0, 0, 0);
        } else {
            createButtonColor(renderer, *passwordInput, "INSERT PASSWORD HERE", 0, 0, 0);
        }
    }

    updateRenderer(renderer);
}

void auth_str_to_uint16(const char *str, uint16_t *res) {
    char *end;
    long val = strtol(str, &end, 10);
    *res = (uint16_t)val;
}

void initAuthenticationInputButtons(){
    usernameInput = malloc(sizeof(Button));
    if(usernameInput==NULL){
        SDL_ExitWithError("ERROR ALLOCATING USERNAME INPUT BUTTON");
    }
    usernameInput->beginX = 100;
    usernameInput->beginY = 90;
    usernameInput->endX = 400;
    usernameInput->endY = 140;

    passwordInput = malloc(sizeof(Button));
    if(passwordInput==NULL){
        SDL_ExitWithError("ERROR ALLOCATING PASSWORD INPUT BUTTON");
    }
    passwordInput->beginX = 100;
    passwordInput->beginY = 250;
    passwordInput->endX = 400;
    passwordInput->endY = 300;
}