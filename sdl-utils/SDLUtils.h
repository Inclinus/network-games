//
// Created by noam on 25/01/23.
//

// SDL Utils related to main window and renderer
void initSDL();
void quitSDL();
void createWindowAndRenderer();
void updateRenderer();

// SDL Utils to draw
void createFilledRectangle(int x, int y, int w, int h);
void changeColor(int red, int green, int blue);

// SDL Error utils
void SDL_ExitWithError(const char *message);
void closeWindowAndRendererAndThrowError(const char *message);
