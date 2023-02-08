gcc server.c tictactoe/tictactoe.c bdd/database.c $(mysql_config --libs) -pthread -o a.out $(mysql_config --cflags)
