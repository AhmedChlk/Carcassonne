#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "engine.h"
#include "controller.h"
#include "gui.h"


int main(void) {
    game_t game = init_game(10,10,"data/tiles.csv");
    game_loop(&game);
    return 0;
}
