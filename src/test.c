#include <stdlib.h>
#include "engine.h"
#include "gui.h"

int main(void) {
    game_t game = init_game(144, 144, "tiles.csv");
    position_t x ={72,71};
    place_tile(game.board,game.frontier,pick_tile(&game.tile_pack),x);
    run_gui(game.board);
    free_board(game.board);
    free_frontier(&game.frontier);
    free_valid_positions(&game.valid_positions);
    return EXIT_SUCCESS;
}
