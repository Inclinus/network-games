void loadMainMenu(SDL_bool * program_launched, int *socketClient);
int choseGameMenu(SDL_Renderer * rendererMenu);
int authentLoginOrRegister(SDL_Renderer * rendererMain);
void createMenuMain(SDL_Renderer * rendererMenu);
void * sdlClientListen();
void *networkMenuListen(int *clientSocket);

void creditMenu(SDL_Renderer * rendererMenu);
void optionsMenu(SDL_Renderer * rendererMenu);

typedef struct{
    int beginX;
    int beginY;
    int endX;
    int endY;
    int actionType;
} Button ;

#define WIDTH 720
#define HEIGHT 480


