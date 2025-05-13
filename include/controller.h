#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "engine.h"
#include "gui.h"
#include <stdbool.h>

/* =========== CONSTANTES DE VUE =========== */

#define VIEW_WIDTH  10
#define VIEW_HEIGHT 5

/* =========== BOUCLE PRINCIPALE =========== */

/** Boucle principale du jeu */
void game_loop(game_t *game);

/* =========== GESTION DE TOUR ET D'UTILS =========== */

/** Passe au joueur suivant */
void next_player(int *current, int total);

/** Renvoie le caractère de commande clavier saisi */
char get_input_key(void);

/** Vérifie s’il existe au moins une option de placement meeple possible */
bool has_any_option(meeple_options_t opts);

/** Affiche le ou les gagnants en fin de partie */
void display_winner(game_players_t players);

/* =========== CURSEUR & PLACEMENT =========== */

/** Gère le déplacement du curseur avec les touches ZQSD / flèches */
bool handle_cursor_movement(char cmd, position_t *cursor, int *viewX, int *viewY, game_t *game);

/** Gère la pioche d’une tuile si disponible */
bool handle_tile_pick(game_t *game, bool *tileInHand, tile_t *tile);

/** Vérifie et place une tuile sur le plateau */
bool handle_tile_placement(game_t *game, position_t cursor, tile_t tile, degree_t rot);

/** Affiche les options meeple et gère le placement */
void handle_meeple_phase(game_t *game,position_t pos,bool *tileInHand,bool *awaitingMeeple,meeple_options_t opts,player_t *p);

#endif // CONTROLLER_H
