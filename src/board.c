#include <stdio.h>
#include <stdlib.h>
#include "board.h"

board_t* alloc_board(int height,int width){
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