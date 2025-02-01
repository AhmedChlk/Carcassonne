#include <stdlib.h>
#include "board.h"

board_t* setup_board(int height,int width){
    board_t* board = (board_t*) malloc(sizeof(board_t));
    if(board == NULL){
        fprintf(stderr,"erreur d'allocation memoire de : board");
        exit(EXIT_FAILURE);
    }
}