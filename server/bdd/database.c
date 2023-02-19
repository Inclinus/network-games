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

    char salt[BCRYPT_HASHSIZE];
    bcrypt_gensalt(12, salt);
    char encrypted_password[BCRYPT_HASHSIZE];

    if (bcrypt_hashpw(password, salt, encrypted_password) != 0) {
        fprintf(stderr, "%s\n", "Erreur lors de l'encryption du mot de passe");
        return 1;
    }

    char query1[100];
    sprintf(query1, "SELECT COUNT(*) FROM users WHERE login='%s'", username);
    if (mysql_query(con, query1)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        printf("Erreur lors de la vérification de l'utilisateur\n");
        return 2;
    }

    MYSQL_RES *res = mysql_use_result(con);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (atoi(row[0]) == 1) {
        // L'utilisateur existe déjà
        mysql_free_result(res);
        printf("L'utilisateur existe déjà\n");
        return 1;
    }
    mysql_free_result(res);

    char query2[100];
    sprintf(query2, "INSERT INTO users (login, password) VALUES ('%s','%s')", username, encrypted_password);
    if (mysql_query(con, query2)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        printf("Erreur lors de l'insertion de l'utilisateur\n");
        return 2;
    }
    return 0;
}

int checkUser(MYSQL *con, char *username, char *password) {
    char query1[100];
    sprintf(query1, "SELECT COUNT(*) FROM users WHERE login='%s'", username);
    if (mysql_query(con, query1)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return 0;
    }
    MYSQL_RES *res1 = mysql_use_result(con);
    MYSQL_ROW row1 = mysql_fetch_row(res1);

    if (atoi(row1[0]) == 0) {
        // L'utilisateur n'existe pas
        mysql_free_result(res1);
        return 0;
    }
    mysql_free_result(res1);

    char query[100];
    sprintf(query, "SELECT password FROM users WHERE login='%s'", username);
    if (mysql_query(con, query)) {
        fprintf(stderr, "%s\n", mysql_error(con));
        return 0;
    }
    MYSQL_RES *res = mysql_use_result(con);
    MYSQL_ROW row = mysql_fetch_row(res);

    printf("HASH : %s\n", row[0]);
    if(bcrypt_checkpw(password, row[0]) == 0){
        // L'utilisateur existe et le mot de passe est correct
        mysql_free_result(res);
        return 1;
    } else {
        // L'utilisateur n'existe pas ou le mot de passe est incorrect
        mysql_free_result(res);
        return 0;
    }
}