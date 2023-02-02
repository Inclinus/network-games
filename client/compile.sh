gcc ../events/EventManager.c client.c tictactoe/tictactoe.c ../sdl-utils/SDLUtils.c $(sdl2-config --cflags --libs) -pthread -lSDL2_ttf -lSDL2_image -o client.out
