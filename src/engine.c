#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include <string.h>
#include "tile.h"
board_t* init_board(int height,int width){
    board_t* board = (board_t*) malloc(sizeof(board_t));
    if(board == NULL){
        fprintf(stderr,"erreur d'allocation memoire de : board");
        exit(EXIT_FAILURE);
    }
    board->height = height;
    board->width = width;
    board->grid = (tile_t**) malloc (height * sizeof (tile_t*));
    for(int i=0;i<height;i++){
        board->grid[i]=(tile_t*)malloc(width * sizeof(tile_t));
    }
    for(int i=0;i<height;i++){
        for(int j=0;j<width;j++){
            board->grid[i][j].is_meeple=0;
            board->grid[i][j].pos_meeple = NO_DIRECTION;
            board->grid[i][j].col_meeple = NO_COLOR;       
            }
    }
    return board;
}
void free_board(board_t* board){
    for(int i=0;i<board->height;i++){
        free(board->grid[i]);
    }
    free(board->grid);
    free(board);
}

tiles_pack_t init_tiles_pack(const char* fname){
    char path[256] = "data/";
    char* token;
    strcat(path,fname);
    FILE* F = fopen(path,"r");
    if(F == NULL){
        fprintf(stderr,"error opening file");
        exit(EXIT_FAILURE);
    }
    int buffer_size = 1024;
    char buffer[buffer_size];
    int index_zones,index_tiles = 0;
    tiles_pack_t tp;
    tp.remaining_tiles = NB_TILES;
    while(fgets(buffer,buffer_size,F) != NULL){
        index_zones = 0;
        token = strtok(buffer,",\n");
        while(token != NULL){
                if (!strcmp(token,"blason"))
                    tp.tab_tiles[index_tiles].tab_zones[index_zones++] = BLASON;

                else if (!strcmp(token,"route"))
                    tp.tab_tiles[index_tiles].tab_zones[index_zones++] = ROAD;
                    
            
                else if (!strcmp(token,"pre"))
                    tp.tab_tiles[index_tiles].tab_zones[index_zones++] = PRE;
                    

                else if (!strcmp(token,"village"))
                    tp.tab_tiles[index_tiles].tab_zones[index_zones++] = VILLAGE;
                    

                else if (!strcmp(token,"ville"))
                    tp.tab_tiles[index_tiles].tab_zones[index_zones++] = CITY;
                else if (!strcmp(token,"abbaye"))
                    tp.tab_tiles[index_tiles].tab_zones[index_zones++] = ABBEY;

            token = strtok(NULL, ",\n");
        }
        index_tiles++;
    }
    return tp;
}





