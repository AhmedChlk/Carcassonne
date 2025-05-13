#ifndef GUI_H
#define GUI_H

#include "board.h"
#include "tile.h"
#include "controller.h" // pour position_t
#include "player.h"     // pour color_t, player_t
#include "engine.h"     // pour game_t

/* =========== AFFICHAGE D'UNE TUILE =========== */

char print_zone(zone_type_t z);
void print_tile(tile_t t);
void gui_show_current_tile(tile_t tile, degree_t rotation);

/* =========== AFFICHAGE DU PLATEAU =========== */

void print_board(board_t *board,int offsetX, int offsetY,int viewW,int viewH,position_t cursor);

/* =========== AFFICHAGE DES INFOS DE TOUR =========== */

void display_turn_header(const char *player_name,color_t col,bool tileInHand,degree_t handRot);
void display_scores(game_t *game);
void display_winner(game_players_t players);

/* =========== MENU ET EN-TÊTE =========== */

int  display_menu(void);
void display_welcome(void);

/* =========== AFFICHAGE DES POSITIONS VALIDE =========== */

void gui_set_valid_positions(valid_positions_t* v);
void gui_display_positions(valid_positions_t* v);

/* =========== AFFICHAGE DES OPTIONS MEEPLE =========== */

void gui_display_meeple_options(meeple_options_t opts, color_t player_color);

#endif // GUI_H
