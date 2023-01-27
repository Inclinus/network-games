void changeColor(int red, int green, int blue){
    if(SDL_SetRenderDrawColor(renderer,red,green,blue,SDL_ALPHA_OPAQUE) != 0)
        SDL_ExitWithError("Unable to change renderer color");
}

void createFilledRectangle(int x, int y, int w, int h){
    SDL_Rect rectangle;
    rectangle.x = x;
    rectangle.y = y;
    rectangle.w = w;
    rectangle.h = h;
    if(SDL_RenderFillRect(renderer, &rectangle) != 0){
        SDL_ExitWithError("Unable to draw rectangle");
    }
}

void createWindowAndRenderer(const char *windowTitle){
    // Window + renderer
    if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH,WINDOW_HEIGHT,0,&window,&renderer) != 0){
        SDL_ExitWithError("Unable to create window and renderer");
    }
    SDL_SetWindowTitle(window,windowTitle);
}

void closeWindowAndRendererAndThrowError(const char *message){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_ExitWithError(message);
}

void updateRenderer(){
    SDL_RenderPresent(renderer);
}

void quitSDL(SDL_Renderer renderer, SDL_Wind){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void initSDL(){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        SDL_ExitWithError("SDL init failed");
    }
}

void SDL_ExitWithError(const char *message){
    SDL_Log("ERREUR : %s > %s\n", message, SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
}
