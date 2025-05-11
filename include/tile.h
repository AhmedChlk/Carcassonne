#ifndef TILE_H
#define TILE_H

#include <stdbool.h>

// === CONSTANTES ===
#define NB_TILES 72  // Nombre total de tuiles


// === ÉNUMÉRATIONS ===

// Types de zones d'une tuile
typedef enum {
    SHIELD, ROAD, FIELD, CITY, VILLAGE, ABBEY, NONE
} zone_type_t;

// Côtés possibles d'une tuile
typedef enum {
    NORTH, EAST, SOUTH, WEST, CENTER, NO_DIRECTION
} direction_t;

// Couleurs possibles pour les meeples
typedef enum {
    RED, BLUE, GREEN, YELLOW, WHITE, NO_COLOR
} color_t;

// Rotations possibles d'une tuile
typedef enum {
    ZERO, NINETY, ONE_HUNDRED_EIGHTY, TWO_HUNDRED_SEVENTY
} degree_t;

// === STRUCTURES DE JEU ===
// Représente une tuile et un meeple éventuel
typedef struct {
    zone_type_t zones[5];
    bool        has_meeple;
    direction_t meeple_position;
    color_t     meeple_color;
} tile_t;

// Pack complet de tuiles
typedef struct {
    tile_t tiles[NB_TILES];
    int    remaining_tiles;
} tiles_pack_t;

// Zones disponibles pour poser un meeple
typedef struct {
    bool available[5];
} meeple_options_t;

// === STRUCTURES DE POSITIONNEMENT ===
typedef struct { int x, y; } position_t;

// Structure pour la frontière de placement
typedef struct {
    position_t* positions;
    int size;       // nombre courant d'éléments
    int capacity;   // capacité maximale allouée
} frontiers_t;

// Liste dynamique de positions valides
typedef struct {
    position_t* positions;
    int size;       // nombre courant de positions valides
    int capacity;   // capacité du tableau
} valid_positions_t;

#endif // TILE_H
