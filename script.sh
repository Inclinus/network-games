#!/bin/bash

echo "Compilation du Serveur ..."
gcc server/server.c server/tictactoe/tictactoe.c server/connect4/connect4.c -pthread -o target/server.out

echo "Compilation du Client ..."
gcc events/EventManager.c sdl-utils/SDLUtils.c client/client.c client/tictactoe/tictactoe.c client/connect4/connect4.c $(sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -lm -pthread  -o target/client.out

echo "Lancement ..."
gnome-terminal --quiet --tab --title="server" --command="bash -c './target/server.out; $SHELL'" --tab --title="client1" --command="bash -c './target/client.out; $SHELL'" --tab --title="client2" --command="bash -c './target/client.out; $SHELL'"

