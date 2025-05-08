#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "tile.h"

// === CONSTANTES ===
#define NB_MEEPLES 7  // Nombre de meeples par joueur

// === STRUCTURE D'UN JOUEUR ===
typedef struct player_t {
    char    name[50];
    int     meeples;
    color_t meeple_color;
    bool    is_ai;
    int     score;
} player_t;

// === STRUCTURE DES JOUEURS DU JEU ===
typedef struct {
    player_t* players;
    int       num_players;
} game_players_t;

#endif // PLAYER_H
