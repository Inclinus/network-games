gcc ../events/EventManager.c client.c tictactoe/tictactoe.c connect4/connect4.c ../sdl-utils/SDLUtils.c $(sdl2-config --cflags --libs) -pthread -lSDL2_ttf -lSDL2_image -lm -o client.out
