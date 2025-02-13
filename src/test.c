#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "engine.h"


char print_zone(zone_type_t z){
    switch(z){
        case SHIELD : 
            return 's';
        case ROAD : 
            return 'r';
        case FIELD : 
            return 'f';
        case CITY:
            return 'c';
        case VILLAGE : 
            return 'v';
        case ABBEY :
            return 'a';
        case NONE : 
            return ' ';
    }
}
void print_tile(tile_t t ){
    printf(" %c \n",print_zone(t.zones[NORTH]));
    printf("%c%c%c\n",print_zone(t.zones[WEST]),print_zone(t.zones[CENTER]),print_zone(t.zones[EAST]));
    printf(" %c \n",print_zone(t.zones[SOUTH]));
}


void print_board(board_t* b) {
    // Affichage de l'en-tête des colonnes
    printf("    "); // Espace réservé pour les numéros de ligne
    for (int j = 0; j < b->width; j++) {
        // Chaque numéro de colonne est centré dans un champ de largeur 3
        printf(" %d ", j);
    }
    printf("\n");

    // Pour chaque ligne du plateau
    for (int i = 0; i < b->height; i++) {
        // Ligne supérieure de la tuile (zone NORTH)
        printf("    "); // Pas de numéro de ligne sur cette ligne
        for (int j = 0; j < b->width; j++) {
            printf(" %c ", print_zone(b->grid[i][j].zones[NORTH]));
        }
        printf("\n");

        // Ligne centrale de la tuile (zones WEST, CENTER, EAST)
        // On affiche le numéro de ligne à gauche, aligné avec le centre de la tuile
        printf("%2d  ", i);
        for (int j = 0; j < b->width; j++) {
            printf("%c%c%c",
                   print_zone(b->grid[i][j].zones[WEST]),
                   print_zone(b->grid[i][j].zones[CENTER]),
                   print_zone(b->grid[i][j].zones[EAST]));
        }
        printf("\n");

        // Ligne inférieure de la tuile (zone SOUTH)
        printf("    ");
        for (int j = 0; j < b->width; j++) {
            printf(" %c ", print_zone(b->grid[i][j].zones[SOUTH]));
        }
        printf("\n");
    }
}


int main() {
    srand(time(NULL));
    board_t* b = initialize_board(5,5);
    frontiers_t*f = allocate_frontier();
    valid_positions_t* v = allocate_valid_positions();
    assert(v != NULL);
    tiles_pack_t tp = initialize_tile_pack("tiles.csv");
    shuffle_tile_pack(&tp);
    tile_t t = pick_tile(&tp);
    t = pick_tile(&tp);
    place_tile(b,f,t,(position_t){1,2});
    t = pick_tile(&tp);
    place_tile(b,f,t,(position_t){2,2});
    t = pick_tile(&tp);
    print_board(b);
    printf("tile a jouer : \n");
    print_tile(t);

    printf("les frontieres :");
    for(int i=0;i<f->actual;i++){
        printf("(%d,%d) ",f->positions[i].x,f->positions[i].y);
    }
    printf("\n");
    get_placement_tile(b,f,t,v);
    printf("placement possible : ");
    for(int i=0;i<v->actual;i++){
        printf("(%d,%d)",v->valid_positions[i].x,v->valid_positions[i].y);
    }
    printf("\n");
    return 0;
}
