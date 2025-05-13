#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "engine.h"
#include "controller.h"
#include "gui.h"


int main(void) {
    game_t game = init_game(11,11,"data/test.csv");
    game_loop(&game);
    free_game(&game);
    return 0;
}
