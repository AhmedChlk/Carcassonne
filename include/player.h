#ifndef PLAYER_H
#define PLAYER_H
#include <stdbool.h>
#define NB_MEEPLES 7
    typedef struct{
        char name[50];
        int meeples;
        color_t col_meeple;
        bool is_ia;
    }player_t;
    
    typedef struct{
        player_t* tab_players;
        int nb_players;
    }game_players_t;
#endif