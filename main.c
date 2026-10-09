#include <stdio.h>

#include "sdl_wrapper.h"
#include "game.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE  "Idle Heroes Manager"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (SDL_Wrapper_Init(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE) != 0) {
        fprintf(stderr, "Fehler: SDL konnte nicht initialisiert werden.\n");
        return 1;
    }

    GameMain();

    SDL_Wrapper_Quit();

    return 0;
}
