#!/bin/bash

echo "Compilation du Serveur ..."
gcc server/server.c server/tictactoe/tictactoe.c -pthread -o target/server.out

echo "Compilation du Client ..."
gcc events/EventManager.c client/client.c client/tictactoe/tictactoe.c sdl-utils/SDLUtils.c $(sdl2-config --cflags --libs) -lSDL2_image -lSDL2_ttf -lm -pthread  -o target/client.out

echo "Lancement ..."
gnome-terminal --quiet --tab --title="server" --command="bash -c './target/server.out; $SHELL'" --tab --title="client1" --command="bash -c './target/client.out; $SHELL'" --tab --title="client2" --command="bash -c './target/client.out; $SHELL'"
