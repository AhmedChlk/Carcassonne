#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

#include "engine.h"
#include "controller.h"
#include "gui.h"
#include "colors.h"

#define BOARD_SIZE 5

// Activez les tests ici
#define ENABLE_TEST_1
//#define ENABLE_TEST_2
//#define ENABLE_TEST_3
//#define ENABLE_TEST_4
//#define ENABLE_TEST_5

// ---------- UTILS ----------

tile_t* create_tile(zone_type_t zones[5]) {
    tile_t* t = (tile_t*) malloc(sizeof(tile_t));
    memset(t, 0, sizeof(tile_t));
    for (int i = 0; i < 5; i++) {
        t->zones[i] = zones[i];
    }
    return t;
}

void free_tile(tile_t* tile) {
    if (tile) free(tile);
}

void place_meeple_test(board_t* board, direction_t dir, position_t pos, color_t color) {
    tile_t* tile = &board->grid[pos.x][pos.y];
    tile->has_meeple      = true;
    tile->meeple_position = dir;
    tile->meeple_color    = color;
}

void print_available_directions(const char* label, meeple_options_t opt, tile_t* tile) {
    const char *names[5] = {"Nord", "Est", "Sud", "Ouest", "Centre"};
    printf("\n\033[1;36mTUILE %s\033[0m\n", label);
    printf("Zones : ");
    for (int i = 0; i < 5; i++) {
        printf("%s=%d ", names[i], tile->zones[i]);
    }
    printf("\nOptions de placement : ");
    bool any = false;
    for (int i = 0; i < 5; i++) {
        if (opt.available[i]) {
            printf("\033[0;32m✔%s\033[0m ", names[i]);
            any = true;
        }
    }
    if (!any) printf("\033[0;31mAucune\033[0m");
    printf("\n");
}

// ---------- TESTS ----------

#ifdef ENABLE_TEST_1
void test_placement_deja_occupe() {
    zone_type_t road[5] = {FIELD, ROAD, FIELD, ROAD, ROAD};
    tile_t *t1 = create_tile(road);
    tile_t *t2 = create_tile(road);

    board_t* board = create_board(BOARD_SIZE, BOARD_SIZE);
    position_t p1 = {1, 1}, p2 = {1, 2};

    board->grid[p1.x][p1.y] = *t1;
    board->grid[p2.x][p2.y] = *t2;

    place_meeple_test(board, WEST, p2, RED);

    meeple_options_t opt = get_placement_meeple(board, p1);
    print_board(board, 0, 0, BOARD_SIZE, BOARD_SIZE, p1);
    print_available_directions("Test 1.1", opt, t1);

    for (int i = 0; i < 5; i++) assert(!opt.available[i]);
    printf(ANSI_GREEN "\n✅ Test 1 validé : structure déjà occupée\n" ANSI_RESET);
    destroy_board(board); free_tile(t1); free_tile(t2);
}
#endif

#ifdef ENABLE_TEST_2
void test_structure_similaire_non_connectee() {
    zone_type_t z1[5] = {FIELD, CITY, FIELD, FIELD, FIELD};
    tile_t *t1 = create_tile(z1);
    tile_t *t2 = create_tile(z1);

    board_t* board = create_board(BOARD_SIZE, BOARD_SIZE);
    position_t p1 = {1, 1}, p2 = {1, 3};
    board->grid[p1.x][p1.y] = *t1;
    board->grid[p2.x][p2.y] = *t2;

    place_meeple_test(board, EAST, p1, BLUE);
    meeple_options_t opt = get_placement_meeple(board, p2);
    print_board(board, 0, 0, BOARD_SIZE, BOARD_SIZE, p1);
    print_available_directions("Test 2.1", opt, t2);

    bool has_option = false;
    for (int i = 0; i < 5; i++) has_option |= opt.available[i];
    assert(has_option);
    printf(ANSI_GREEN "\n✅ Test 2 validé : structures similaires non connectées\n" ANSI_RESET);
    destroy_board(board); free_tile(t1); free_tile(t2);
}
#endif

#ifdef ENABLE_TEST_3
void test_fusion_structures() {
    zone_type_t z_half[5] = {FIELD, ROAD, FIELD, ROAD, ROAD};
    zone_type_t z_mid[5] = {ROAD, ROAD, ROAD, ROAD, ROAD};

    tile_t *tl = create_tile(z_half);
    tile_t *tr = create_tile(z_half);
    tile_t *tm = create_tile(z_mid);

    board_t* board = create_board(BOARD_SIZE, BOARD_SIZE);
    position_t pl = {2, 1}, pr = {2, 3}, pm = {2, 2};

    board->grid[pl.x][pl.y] = *tl;
    board->grid[pr.x][pr.y] = *tr;
    board->grid[pm.x][pm.y] = *tm;

    place_meeple_test(board, EAST, pl, RED);
    place_meeple_test(board, WEST, pr, BLUE);

    meeple_options_t opt = get_placement_meeple(board, pm);
    print_board(board, 0, 0, BOARD_SIZE, BOARD_SIZE, pm);
    print_available_directions("Test 3.1", opt, tm);

    for (int i = 0; i < 5; i++) assert(!opt.available[i]);
    printf(ANSI_GREEN "\n✅ Test 3 validé : fusion bloquée\n" ANSI_RESET);
    destroy_board(board); free_tile(tl); free_tile(tr); free_tile(tm);
}
#endif

#ifdef ENABLE_TEST_4
void test_placement_centre_uniquement() {
    zone_type_t abbaye[5] = {FIELD, FIELD, FIELD, FIELD, ABBEY};
    tile_t *t = create_tile(abbaye);

    board_t* board = create_board(BOARD_SIZE, BOARD_SIZE);
    position_t p = {2, 2};
    board->grid[p.x][p.y] = *t;

    meeple_options_t opt = get_placement_meeple(board, p);
    print_board(board, 0, 0, BOARD_SIZE, BOARD_SIZE, p);
    print_available_directions("Test 4.1", opt, t);

    assert(opt.available[CENTER]);
    for (int i = 0; i < 4; i++) assert(!opt.available[i]);
    printf(ANSI_GREEN "\n✅ Test 4 validé : placement centre uniquement\n" ANSI_RESET);
    destroy_board(board); free_tile(t);
}
#endif

#ifdef ENABLE_TEST_5
void test_double_meeple_bloque() {
    zone_type_t z1[5] = {FIELD, CITY, FIELD, FIELD, FIELD};
    zone_type_t z2[5] = {FIELD, FIELD, FIELD, CITY, FIELD};

    tile_t *t1 = create_tile(z1);
    tile_t *t2 = create_tile(z2);

    board_t* board = create_board(BOARD_SIZE, BOARD_SIZE);
    position_t p1 = {2, 2}, p2 = {2, 3};
    board->grid[p1.x][p1.y] = *t1;
    board->grid[p2.x][p2.y] = *t2;

    place_meeple_test(board, EAST, p1, RED);
    meeple_options_t opt = get_placement_meeple(board, p2);
    print_board(board, 0, 0, BOARD_SIZE, BOARD_SIZE, p2);
    print_available_directions("Test 5.1", opt, t2);

    for (int i = 0; i < 5; i++) assert(!opt.available[i]);
    printf(ANSI_GREEN "\n✅ Test 5 validé : double meeple bloqué\n" ANSI_RESET);
    destroy_board(board); free_tile(t1); free_tile(t2);
}
#endif

// ---------- MAIN ----------

int main(void) {
#ifdef ENABLE_TEST_1
    test_placement_deja_occupe();
#endif
#ifdef ENABLE_TEST_2
    test_structure_similaire_non_connectee();
#endif
#ifdef ENABLE_TEST_3
    test_fusion_structures();
#endif
#ifdef ENABLE_TEST_4
    test_placement_centre_uniquement();
#endif
#ifdef ENABLE_TEST_5
    test_double_meeple_bloque();
#endif
    return 0;
}
