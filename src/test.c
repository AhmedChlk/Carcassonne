#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "engine.h"
#include "gui.h"

#define ANSI_RED    "\x1b[31m"
#define ANSI_BLUE   "\x1b[34m"
#define ANSI_GREEN  "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_WHITE  "\x1b[37m"
#define ANSI_RESET  "\x1b[0m"

const char *ANSI_COLOR_CODES[] = {
    [RED]      = ANSI_RED,
    [BLUE]     = ANSI_BLUE,
    [GREEN]    = ANSI_GREEN,
    [YELLOW]   = ANSI_YELLOW,
    [WHITE]    = ANSI_WHITE,
    [NO_COLOR] = ANSI_RESET
};

void print_board(board_t* b) {
    printf("    ");
    for (int j = 0; j < b->width; j++) printf(" %d  ", j);
    printf("\n");
    for (int i = 0; i < b->height; i++) {
        // North
        printf("    ");
        for (int j = 0; j < b->width; j++) {
            tile_t* t = &b->grid[i][j];
            char c = print_zone(t->zones[NORTH]);
            printf(" %s%c%s  ", t->has_meeple && t->meeple_position==NORTH
                   ? ANSI_COLOR_CODES[t->meeple_color] : "", c,
                   t->has_meeple && t->meeple_position==NORTH
                   ? ANSI_RESET : "");
        }
        printf("\n");
        // West-Center-East
        printf("%2d  ", i);
        for (int j = 0; j < b->width; j++) {
            tile_t* t = &b->grid[i][j];
            direction_t dirs[3] = { WEST, CENTER, EAST };
            for (int d = 0; d < 3; d++) {
                char ch = print_zone(t->zones[dirs[d]]);
                if (t->has_meeple && t->meeple_position==dirs[d])
                    printf("%s%c%s", ANSI_COLOR_CODES[t->meeple_color], ch, ANSI_RESET);
                else
                    printf("%c", ch);
            }
            printf(" ");
        }
        printf("\n");
        // South
        printf("    ");
        for (int j = 0; j < b->width; j++) {
            tile_t* t = &b->grid[i][j];
            char c = print_zone(t->zones[SOUTH]);
            printf(" %s%c%s  ", t->has_meeple && t->meeple_position==SOUTH
                   ? ANSI_COLOR_CODES[t->meeple_color] : "", c,
                   t->has_meeple && t->meeple_position==SOUTH
                   ? ANSI_RESET : "");
        }
        printf("\n");
    }
    printf("\n");
}

static tile_t create_tile(zone_type_t n, zone_type_t e, zone_type_t s, zone_type_t w,
                          zone_type_t c, bool has, direction_t dir, color_t col) {
    tile_t t;
    t.zones[NORTH] = n;
    t.zones[EAST]  = e;
    t.zones[SOUTH] = s;
    t.zones[WEST]  = w;
    t.zones[CENTER]= c;
    t.has_meeple     = has;
    t.meeple_position= dir;
    t.meeple_color   = col;
    return t;
}

game_players_t create_test_players(void) {
    game_players_t gp;
    gp.num_players = 2;
    gp.players = malloc(2 * sizeof(player_t));
    strcpy(gp.players[0].name, "Bob");
    gp.players[0].meeples = NB_MEEPLES;
    gp.players[0].meeple_color = RED;
    gp.players[0].is_ai = false;
    gp.players[0].score = 0;
    strcpy(gp.players[1].name, "Alice");
    gp.players[1].meeples = NB_MEEPLES;
    gp.players[1].meeple_color = GREEN;
    gp.players[1].is_ai = false;
    gp.players[1].score = 0;
    return gp;
}

void print_player_scores_and_meeples(game_players_t* gp) {
    for (int i = 0; i < gp->num_players; i++) {
        printf("Joueur %s : %d meeples, score %d\n",
               gp->players[i].name, gp->players[i].meeples, gp->players[i].score);
    }
}


int main(void) {
    game_t game;
    game.board = initialize_board(5, 5);
    game.frontier = allocate_frontier();
    game.valid_positions = allocate_valid_positions();
    game.tile_pack = initialize_tile_pack("tiles.csv");
    game.players = create_test_players();

    //TEST ROUTES FERMEES
    // Test 1 : Route fermée avec un meeple
    /*
    tile_t t1 = create_tile(ROAD, ROAD, CITY, CITY, CITY, true, NORTH, RED);
    tile_t t2 = create_tile(CITY, CITY, ROAD, CITY, CITY, false, NO_DIRECTION, NO_COLOR);
    tile_t t3 = create_tile(CITY, ROAD, ROAD, CITY, ROAD, false, NO_DIRECTION, NO_COLOR);
    tile_t t4 = create_tile(CITY, CITY, CITY, ROAD, ROAD, true, WEST, GREEN);
    place_tile(game.board, game.frontier, t1, (position_t){2, 2});
    printf("Test Route Fermé 1 meeple :\n");
    printf("Fermeture de la structure en (1,2)\n");
    place_tile(game.board, game.frontier, t2, (position_t){1, 2});
    printf("avant appele de score_completed_structures : \n");
    print_player_scores_and_meeples(&game.players);
    print_board(game.board);
    score_completed_structures(&game, (position_t){1, 2});
    printf("apres appele de score_completed_structures : \n");
    print_player_scores_and_meeples(&game.players);
    print_board(game.board);
    printf("Test 1 passé\n");
    */
    /*
    printf("Test Route Fermé 2 meeple :\n");
    place_tile(game.board, game.frontier, t1, (position_t){2, 2});
    place_tile(game.board, game.frontier, t2, (position_t){1, 2});
    place_tile(game.board, game.frontier, t3, (position_t){1, 2});
    place_tile(game.board, game.frontier, t4, (position_t){1, 3});
    printf("avant appele de score_completed_structures : \n");
    print_player_scores_and_meeples(&game.players);
    print_board(game.board);
    // Fermeture de la structure en (1,2)
    score_completed_structures(&game, (position_t){1, 2});
    printf("apres appele de score_completed_structures : \n");
    print_player_scores_and_meeples(&game.players);
    print_board(game.board);
    printf("Test 2 passé\n");
    */
    //TEST ABBEYES
    // Test 3 : Abbaye fermée avec un meeple
    /*
    tile_t abb = create_tile(FIELD, FIELD, FIELD, FIELD, ABBEY, false, NO_DIRECTION, NO_COLOR);
    tile_t f = create_tile(FIELD, FIELD, FIELD, FIELD, FIELD, false, NO_DIRECTION, NO_COLOR);
    place_tile(game.board, game.frontier, abb, (position_t){2, 2});
    place_tile(game.board, game.frontier, f, (position_t){1, 1});
    place_tile(game.board, game.frontier, f, (position_t){1, 2});
    place_tile(game.board, game.frontier, f, (position_t){1, 3});
    place_tile(game.board, game.frontier, f, (position_t){2, 1});
    place_tile(game.board, game.frontier, f, (position_t){2, 3});
    place_tile(game.board, game.frontier, f, (position_t){3, 1});
    place_tile(game.board, game.frontier, f, (position_t){3, 2});
    place_tile(game.board, game.frontier, f, (position_t){3, 3});
    printf("Test Abbaye Fermée 1 meeple :\n");
    place_meeple(game.board, &game.players, CENTER, (position_t){2, 2}, RED);
    printf("avant appele de score_completed_structures : \n");
    print_board(game.board);
    print_player_scores_and_meeples(&game.players);
    //score_completed_structures(&game, (position_t){3, 3});
    //score_completed_structures(&game, (position_t){1, 1});
    //score_completed_structures(&game, (position_t){1, 2});
    //score_completed_structures(&game, (position_t){1, 3});
    //score_completed_structures(&game, (position_t){2, 1});
    score_completed_structures(&game, (position_t){2, 2});
    //score_completed_structures(&game, (position_t){2, 3});
    //sore_completed_structures(&game, (position_t){3, 1});
    //score_completed_structures(&game, (position_t){3, 2});
    //score_completed_structures(&game, (position_t){3, 3});
    printf("apres appele de score_completed_structures : \n");
    print_board(game.board);
    print_player_scores_and_meeples(&game.players);
    printf("Test 3 passé\n");
    */
    //TEST combinaisons
    // Test 4 : Abbaye fermée avec un meeple + route fermée
    /*
    tile_t r = create_tile(FIELD, FIELD, ROAD, FIELD, ROAD, false, NO_DIRECTION, NO_COLOR);
    tile_t r2 = create_tile(ROAD, FIELD, FIELD, FIELD, ROAD, false, NO_DIRECTION, NO_COLOR);
    place_tile(game.board, game.frontier, abb, (position_t){2, 2});
    place_tile(game.board, game.frontier, f, (position_t){1, 1});
    place_tile(game.board, game.frontier, f, (position_t){1, 2});
    place_tile(game.board, game.frontier, f, (position_t){1, 3});
    place_tile(game.board, game.frontier, f, (position_t){2, 1});
    place_tile(game.board, game.frontier, f, (position_t){2, 3});
    place_tile(game.board, game.frontier, f, (position_t){3, 1});
    place_tile(game.board, game.frontier, f, (position_t){3, 2});
    place_tile(game.board, game.frontier, r, (position_t){3, 3});
    place_tile(game.board, game.frontier, r2, (position_t){4, 3});
    
    printf("Test Abbaye Fermée 1 meeple :\n");
    place_meeple(game.board, &game.players, CENTER, (position_t){2, 2}, RED);
    place_meeple(game.board, &game.players, SOUTH, (position_t){3, 3}, GREEN);
    printf("avant appele de score_completed_structures : \n");
    print_board(game.board);
    print_player_scores_and_meeples(&game.players);
    printf("apres appele de score_completed_structures : \n");
    score_completed_structures(&game, (position_t){3, 3});
    print_board(game.board);
    print_player_scores_and_meeples(&game.players);
    printf("Test 4 passé\n");
    */
    // Test 5 : Route fermée sans meeple
    /*
    tile_t r = create_tile(FIELD, FIELD, ROAD, FIELD, ROAD, false, NO_DIRECTION, NO_COLOR);
    tile_t r2 = create_tile(ROAD, FIELD, FIELD, FIELD, ROAD, false, NO_DIRECTION, NO_COLOR);
    place_tile(game.board, game.frontier, r, (position_t){3, 3});
    place_tile(game.board, game.frontier, r2, (position_t){4, 3});
    printf("avant appele de score_completed_structures : \n");
    print_board(game.board);
    print_player_scores_and_meeples(&game.players);
    printf("apres appele de score_completed_structures : \n");
    score_completed_structures(&game, (position_t){3, 3});
    print_board(game.board);
    print_player_scores_and_meeples(&game.players);
    printf("Test 4 passé\n");
    */
    // Test 6 : City fermée avec un meeple
    tile_t c = create_tile(FIELD, FIELD, CITY, FIELD, CITY, false, NO_DIRECTION, NO_COLOR);
    //tile_t c2 = create_tile(CITY, FIELD, FIELD, FIELD, CITY, false, NO_DIRECTION, NO_COLOR);
    //test avec shield
    tile_t c2 = create_tile(CITY, FIELD, FIELD, FIELD, SHIELD, false, NO_DIRECTION, NO_COLOR);

    place_tile(game.board, game.frontier, c, (position_t){3, 3});
    place_tile(game.board, game.frontier, c2, (position_t){4, 3});
    place_meeple(game.board, &game.players, SOUTH, (position_t){3, 3}, GREEN);
    printf("avant appele de score_completed_structures : \n");
    print_board(game.board);
    print_player_scores_and_meeples(&game.players);
    printf("apres appele de score_completed_structures : \n");
    score_completed_structures(&game, (position_t){3, 3});
    print_board(game.board);
    print_player_scores_and_meeples(&game.players);
    printf("Test 6 passé\n");
    printf("Tous les tests passent !\n");
    // Libération de la mémoire
    free_game(&game);
    return EXIT_SUCCESS;
}

