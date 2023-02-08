#include <mysql/mysql.h>

#ifndef NETWORK_GAMES_DATABASE_H
#define NETWORK_GAMES_DATABASE_H

MYSQL * connectBdd();

int closeBdd(MYSQL *con);

int checkUser(MYSQL *con, char *username, char *password);

#endif