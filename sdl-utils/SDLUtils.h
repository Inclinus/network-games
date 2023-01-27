// SDL Utils related to main window and renderer
void initSDL();
void quitSDL(SDL_Renderer * renderer, SDL_Window * window);
void updateRenderer(SDL_Renderer * renderer);

// SDL Utils to draw
void createFilledRectangle(int x, int y, int w, int h, SDL_Renderer * renderer);
void changeColor(SDL_Renderer * renderer,int red, int green, int blue);

// SDL Error utils
void SDL_ExitWithError(const char *message);
void closeWindowAndRendererAndThrowError(SDL_Renderer * renderer, SDL_Window * window, const char *message);
