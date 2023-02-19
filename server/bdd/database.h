#include <mysql/mysql.h>

#ifndef NETWORK_GAMES_DATABASE_H
#define NETWORK_GAMES_DATABASE_H

typedef struct Stats {
    int nbWinTictactoe;
    int nbLooseTictactoe;
    int nbDrawTictactoe;
    int nbWinConnect4;
    int nbLooseConnect4;
    int nbDrawConnect4;
} Stats;

MYSQL * connectBdd();

int closeBdd(MYSQL *con);

int createUser(MYSQL *con, char *username, char *password);

int connectUser(MYSQL *con, char *username, char *password);

char * getUsernameConnected();

Stats getStats(MYSQL *con, char *username);

int addStats(MYSQL *con, char *username, int action);

#endif