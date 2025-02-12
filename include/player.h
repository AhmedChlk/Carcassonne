#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "tile.h"
#define NB_MEEPLES 7

typedef struct {
    char name[50];
    int meeples;
    color_t meeple_color;
    bool is_ai;
} player_t;

typedef struct {
    player_t* players;
    int num_players;
} game_players_t;

#endif
