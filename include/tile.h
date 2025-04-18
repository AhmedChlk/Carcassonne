
#ifndef TILE_H
#define TILE_H

#include <stdbool.h>
#define NB_TILES 72
#define ANSI_RED    "\x1b[31m"
#define ANSI_BLUE   "\x1b[34m"
#define ANSI_GREEN  "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_WHITE  "\x1b[37m"
#define ANSI_RESET  "\x1b[0m"


typedef struct {
    int x, y;
} position_t;

typedef struct {
    position_t* valid_positions;
    int actual, capacity;
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

static const char *ANSI_COLOR_CODES[] = {
    [RED]      = ANSI_RED,
    [BLUE]     = ANSI_BLUE,
    [GREEN]    = ANSI_GREEN,
    [YELLOW]   = ANSI_YELLOW,
    [WHITE]    = ANSI_WHITE,
    [NO_COLOR] = ANSI_RESET
};

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


/*
   Structure pour stocker les zones d’une tuile où l’on peut poser le meeple.
   La valeur true indique que le placement est autorisé pour cette zone.
*/
typedef struct {
    bool available[5];
} meeple_options_t;

#endif

