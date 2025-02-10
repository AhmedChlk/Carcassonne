#ifndef TILE_H
#define TILE_H
#define NB_TILES 72
#include<stdbool.h>

    typedef struct{
        int x,y;
    }position_t;
    
    typedef struct{
        position_t* tab_valide_pos;
        int taille,actual;
    }valide_position_t;

    typedef struct{
        position_t* tab_frontieres;
        int actual,capacity;
    }frontieres_t;

    typedef enum{
        BLASON,
        ROAD,
        PRE,
        CITY,
        VILLAGE,
        ABBEY,
        NO_ZONE
    }type_zone_t;

    typedef enum{
        NORTH,
        EAST,
        SOUTH,
        WEST,
        CENTER,
        NO_DIRECTION
    }direction_t;

    typedef enum{
        RED,
        BLEUE,
        GREEN,
        YELLOW,
        WHITE,
        NO_COLOR
    }color_t;

    typedef enum {
        ZERO,
        NINETY,
        ONE_HUNDRED_EIGHTY,
        TWO_HUNDRED_SEVENTY
    }degree_t;

    typedef struct{
        type_zone_t tab_zones[5];
        bool is_meeple;
        direction_t pos_meeple;
        color_t col_meeple;
    }tile_t;

    typedef struct{
        tile_t tab_tiles[NB_TILES];
        int remaining_tiles;
    }tiles_pack_t;
#endif