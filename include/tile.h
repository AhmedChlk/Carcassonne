#ifndef TILE_H
#define TILE_H
#define NB_TILES 72

    typedef enum{
        BLASON,
        ROAD,
        PRE,
        CITY,
        VILLAGE,
        ABBEY
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

    typedef struct{
        type_zone_t tab_zones[5];
        int is_meeple;
        direction_t pos_meeple;
        color_t col_meeple;
    }tile_t;

    typedef struct{
        tile_t tab_tiles[NB_TILES];
        int remaining_tiles;
    }tiles_pack_t;
#endif