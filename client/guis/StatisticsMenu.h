#include "GUI.h"

typedef struct Stats {
    int nbWinTictactoe;
    int nbLooseTictactoe;
    int nbDrawTictactoe;
    int nbWinConnect4;
    int nbLooseConnect4;
    int nbDrawConnect4;
} Stats;


void statisticsMenu(const int * statisticsSocketClient,SDL_Renderer * rendererMenu, Stats * stats);