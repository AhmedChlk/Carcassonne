#ifndef TILE_H
#define TILE_H

    typedef enum{
        BLASON,
        ROAD,
        PRE,
        CITY,
        VILLAGE,
        ABBEYE,
    }type_zone_t;

    typedef enum{
        NORTH,
        EAST,
        SOUTH,
        WEST,
        CENTER,
        NOTHING
    }direction_t;

    typedef enum{
        RED,
        BLEUE,
        GREEN,
        YELLOW,
        WHITE,
        NOTHING
    }color_t;

    typedef struct{
        type_zone_t tab_zones[5];
        int is_meeple;
        direction_t pos_meeple;
        color_t col_meeple;
    }tile_t;
#endif