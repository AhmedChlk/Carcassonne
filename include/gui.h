#ifndef GUI_H
#define GUI_H

#include "board.h"
#include "tile.h"
#include "controller.h"

// === AFFICHAGE DES TUILES ET ZONES ===
char print_zone(zone_type_t z);
void print_tile(tile_t t);
void gui_show_current_tile(tile_t tile, degree_t rotation);

// === AFFICHAGE DES POSITIONS VALIDES ===
void gui_display_positions(valid_positions_t* v);
void gui_set_valid_positions(valid_positions_t* v);

// === AFFICHAGE DES OPTIONS DE MEEPLE ===
void gui_display_meeple_options(meeple_options_t opts, color_t player_color);

// === AFFICHAGE DU NOMBRE DE TUILES RESTANTES ===
void gui_set_remaining_tiles(int remaining);

// === AFFICHAGE DU PLATEAU ===
/**
 * \brief Affiche une fenêtre paginée 10×5 du plateau 144×144,
 *        surligne en vert les cases valides et reverse-video la case du curseur.
 */
void print_board(board_t *board,
                 int offsetX, int offsetY,
                 int viewW,   int viewH,
                 position_t cursor);

#endif // GUI_H
