#ifndef TILE_H
#define TILE_H

#include <stdbool.h>
#define NB_TILES 72

typedef struct {
    int x, y;
} position_t;

typedef struct {
    position_t* valid_positions;
    int size, count;
} valid_positions_t;

typedef struct {
    position_t* positions;
    int actual, capacity;
} frontiers_t;

typedef enum {
    SHIELD,
    ROAD,
    FIELD,
    CITY,
    VILLAGE,
    ABBEY,
    NONE
} zone_type_t;

typedef enum {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    CENTER,
    NO_DIRECTION
} direction_t;

typedef enum {
    RED,
    BLUE,
    GREEN,
    YELLOW,
    WHITE,
    NO_COLOR
} color_t;

typedef enum {
    ZERO,
    NINETY,
    ONE_HUNDRED_EIGHTY,
    TWO_HUNDRED_SEVENTY
} degree_t;

typedef struct {
    zone_type_t zones[5];
    bool has_meeple;
    direction_t meeple_position;
    color_t meeple_color;
} tile_t;

typedef struct {
    tile_t tiles[NB_TILES];
    int remaining_tiles;
} tiles_pack_t;

#endif
