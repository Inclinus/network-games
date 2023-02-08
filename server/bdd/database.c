#include "database.h"
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "../libs/bcrypt/bcrypt.h"
#include <pwd.h>

MYSQL * connectBdd()
{
    MYSQL *con = mysql_init(NULL);

    if (con == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(con));
        return NULL;
    }

    if (mysql_real_connect(con, "localhost", "admin", "eGkE656Sa36yFp",
                           "network_games_bdd", 0, NULL, 0) == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(con));
        return NULL;
    }

    return con;
}

int closeBdd(MYSQL *con)
{
    mysql_close(con);
    return 0;
}

int createUser(MYSQL *con, char *username, char *password) {

    char salt[] = "$2y$10$";
    char encrypted_password[BCRYPT_HASHSIZE];

    if (bcrypt_hashpw(password, salt, encrypted_password) != 0) {
        fprintf(stderr, "%s\n", "Erreur lors de l'encryption du mot de passe");
        return 1;
    }

    char query[100];
    sprintf(query, "INSERT INTO users (login, password) VALUES ('%s','%s')", username, encrypted_password);
    if (mysql_query(con, query)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return 1;
    }
    return 0;
}

int checkUser(MYSQL *con, char *username, char *password) {
    char query[100];
    sprintf(query, "SELECT COUNT(*) FROM users WHERE login='%s' AND password='%s'", username, password);
    if (mysql_query(con, query)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return 0;
    }
    MYSQL_RES *res = mysql_use_result(con);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (atoi(row[0]) == 1) {
        // L'utilisateur existe et le mot de passe est correct
        mysql_free_result(res);
        return 1;
    } else {
        // L'utilisateur n'existe pas ou le mot de passe est incorrect
        mysql_free_result(res);
        return 0;
    }
}