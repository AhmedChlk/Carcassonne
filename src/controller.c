#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "controller.h"
#include "gui.h"
#include "colors.h"

// === Fonctions utilitaires ===
bool has_any_option(meeple_options_t opts) {
    for (int i = 0; i < 5; ++i) {
        if (opts.available[i]) return true;
    }
    return false;
}

void next_player(int *current, int total) {
    *current = (*current + 1) % total;
}


char get_input_key() {
    char buf[4] = {0};
    int n = 0, ch;
    while ((ch = getchar()) != '\n' && n < 4) buf[n++] = ch;
    if (n >= 2 && buf[0] == '\033' && buf[1] == '[') {
        switch (buf[2]) {
            case 'A': return 'U';
            case 'B': return 'D';
            case 'C': return 'R';
            case 'D': return 'L';
        }
    } else if (n == 1) {
        return tolower(buf[0]);
    }
    return '\n';
}

bool handle_cursor_movement(char cmd, position_t *cursor, int *viewX, int *viewY, game_t *game) {
    int dx = 0, dy = 0;
    switch (cmd) {
        case 'z': dx = -1; break;
        case 's': dx = +1; break;
        case 'q': dy = -1; break;
        case 'd': dy = +1; break;
        case 'U': dx = -VIEW_HEIGHT; break;
        case 'D': dx = +VIEW_HEIGHT; break;
        case 'L': dy = -VIEW_WIDTH;  break;
        case 'R': dy = +VIEW_WIDTH;  break;
    }
    if (!dx && !dy) return false;

    int nx = cursor->x + dx;
    int ny = cursor->y + dy;
    if (nx < 0) nx = 0;
    if (nx >= game->board->height) nx = game->board->height - 1;
    if (ny < 0) ny = 0;
    if (ny >= game->board->width) ny = game->board->width - 1;

    cursor->x = nx; cursor->y = ny;

    if (cursor->x < *viewY) *viewY = cursor->x;
    if (cursor->x >= *viewY + VIEW_HEIGHT) *viewY = cursor->x - VIEW_HEIGHT + 1;
    if (cursor->y < *viewX) *viewX = cursor->y;
    if (cursor->y >= *viewX + VIEW_WIDTH) *viewX = cursor->y - VIEW_WIDTH + 1;

    return true;
}

bool handle_tile_pick(game_t *game, bool *tileInHand, tile_t *tile) {
    if (game->tile_pack.remaining_tiles <= 0) return false;
    *tile = pick_tile(&game->tile_pack);
    *tileInHand = true;
    return true;
}

bool handle_tile_placement(game_t *game, position_t cursor, tile_t tile, degree_t rot) {
    for (int i = 0; i < game->valid_positions->size; ++i) {
        position_t vp = game->valid_positions->positions[i];
        if (vp.x == cursor.x && vp.y == cursor.y) {
            for (int r = 0; r < (int)rot; ++r)
                rotate_tile(&tile, NINETY);
            place_tile(game->board, game->frontier, tile, cursor);
            return true;
        }
    }
    return false;
}
void handle_meeple_phase(game_t *game, position_t tile_pos, bool *tileInHand, bool *awaitingMeeple,
                         meeple_options_t opts, player_t *p) {
    while (1) {
        system("clear");
        display_turn_header(p->name, p->meeple_color, *tileInHand, ZERO);
        display_scores(game);
        printf("Tuiles restantes : %d\n\n", game->tile_pack.remaining_tiles);
        print_board(game->board, tile_pos.y - VIEW_WIDTH / 2, tile_pos.x - VIEW_HEIGHT / 2, VIEW_WIDTH, VIEW_HEIGHT, tile_pos);
        gui_display_meeple_options(opts, p->meeple_color);
        printf("Choix Meeple : [N]Nord [S]Sud [E]Est [W]Ouest [C]Centre\n");

        fflush(stdout);
        char d;
        do { d = tolower(getchar()); } while (strchr("nsewc", d) == NULL);
        while (getchar() != '\n');

        direction_t dir = (d == 'n' ? NORTH :
                           d == 's' ? SOUTH :
                           d == 'e' ? EAST  :
                           d == 'w' ? WEST  : CENTER);

        if (opts.available[dir]) {
            place_meeple(game->board, &game->players, dir, tile_pos, p->meeple_color);
            break;
        } else {
            printf("Option invalide.\n");
            sleep(2);
        }
    }

    *awaitingMeeple = false;
    *tileInHand = false;
    score_completed_structures(game, tile_pos);
}

void game_loop(game_t *game) {
    bool game_over = false;
    int current = 0;
    int centerX = game->board->height / 2;
    int centerY = game->board->width / 2;
    int viewX = centerY - VIEW_WIDTH / 2;
    int viewY = centerX - VIEW_HEIGHT / 2;
    if (viewX < 0) viewX = 0;
    if (viewY < 0) viewY = 0;

    position_t cursor = {centerX, centerY};
    bool tileInHand = false, awaitingMeeple = false;
    tile_t handOrigTile;
    degree_t handRot = ZERO;
    meeple_options_t currentOpts;
    position_t lastTilePos;

    while (!game_over) {
        system("clear");
        player_t *p = &game->players.players[current];

        display_turn_header(p->name, p->meeple_color, tileInHand, handRot);
        display_scores(game);
        printf("Tuiles restantes : %d\n\n", game->tile_pack.remaining_tiles);

        if (tileInHand && !awaitingMeeple) {
            bool found = false;
            for (int r = 0; r < 4; ++r) {
                get_placement_tile(game->board, game->frontier, handOrigTile, r, game->valid_positions);
                if (game->valid_positions->size > 0) {
                    handRot = r;
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("Aucune position valide pour cette tuile. Nouvelle pioche...\n");
                sleep(2);
                tileInHand = false;
                continue;
            }
            get_placement_tile(game->board, game->frontier, handOrigTile, handRot, game->valid_positions);
            gui_set_valid_positions(game->valid_positions);
        } else {
            gui_set_valid_positions(NULL);
        }

        print_board(game->board, viewX, viewY, VIEW_WIDTH, VIEW_HEIGHT, cursor);
        if (tileInHand) gui_show_current_tile(handOrigTile, handRot);
        if (!tileInHand) printf("\nZQSD=\u00b11 | \u2191\u2193\u2190\u2192=\u00b110 | Entr\u00e9e=pioche\n");
        else if (!awaitingMeeple) printf("\nZQSD=\u00b11 | \u2191\u2193\u2190\u2192=\u00b110 | R=rotate | Entr\u00e9e=place\n");
        else printf("\nM=choix meeple | N=skip meeple\n");

        char input = get_input_key();
        if (!awaitingMeeple && handle_cursor_movement(input, &cursor, &viewX, &viewY, game)) continue;

        if (!tileInHand && input == '\n') {
            if (handle_tile_pick(game, &tileInHand, &handOrigTile)) continue;
            continue;
        }

        if (tileInHand && !awaitingMeeple) {
            if (input == 'r') {
                handRot = (degree_t)((handRot + 1) % 4);
                get_placement_tile(game->board, game->frontier, handOrigTile, handRot, game->valid_positions);
                continue;
            }
            if (input == '\n') {
                if (!handle_tile_placement(game, cursor, handOrigTile, handRot)) {
                    printf("Position invalide.\n");
                    sleep(2);
                    continue;
                }
                currentOpts = get_placement_meeple(game->board, cursor);
                lastTilePos = cursor;
                if (p->meeples == 0 || !has_any_option(currentOpts)) {
                    printf("Aucun meeple dispo ou aucune position valide. Passage...\n");
                    sleep(2);
                    tileInHand = false;
                    score_completed_structures(game, lastTilePos);
                    next_player(&current, game->players.num_players);
                    continue;
                }
                awaitingMeeple = true;
                continue;
            }
        }

        if (awaitingMeeple) {
            if (input == 'm') {
                handle_meeple_phase(game, lastTilePos, &tileInHand, &awaitingMeeple, currentOpts, p);
                next_player(&current, game->players.num_players);
                continue;
            }
            if (input == 'n') {
                awaitingMeeple = false;
                tileInHand = false;
                score_completed_structures(game, lastTilePos);
                next_player(&current, game->players.num_players);
                continue;
            }
        }

        if (game->tile_pack.remaining_tiles <= 0 && !tileInHand && !awaitingMeeple) {
            score_incomplete_structures(game);
            game_over = true;
        }
    }

    display_winner(game->players);
}
