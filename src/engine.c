#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "board.h"
#include "tile.h"
#include "player.h"

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
        tp.tab_tiles[index_tiles].is_meeple = false;
        tp.tab_tiles[index_tiles].pos_meeple = NO_DIRECTION;
        tp.tab_tiles[index_tiles].col_meeple = NO_COLOR;
        index_tiles++;
    }
    return tp;
}

void shuffle_tiles_pack(tiles_pack_t* tp){
        tile_t temp;
        int tile_1,tile_2;
    for(int i=0;i<NB_TILES;i++){
        do{ 
        tile_1 = rand() % NB_TILES;
        tile_2 = rand() % NB_TILES;
        }while(tile_1 == tile_2);
        temp = tp->tab_tiles[tile_1];
        tp->tab_tiles[tile_1] = tp->tab_tiles[tile_2];
        tp->tab_tiles[tile_2] = temp;
    }   
}

game_players_t create_players(){
    game_players_t gp;
    color_t tab_colors[5] = {RED,BLEUE,GREEN,YELLOW,WHITE};
    int nb_players;
    printf(" a combien de joueurs allez vous jouer (min 1 max 5) : ");
    while(scanf("%d",&nb_players) != 1  || nb_players < 1 || nb_players > 5){
        printf("erreur entrez une valeur valide\n");
        printf(" a combien de joueurs allez vous jouer (min 1 max 5) : ");
        while(getchar() != '\n');
    }
    while(getchar() != '\n');
    gp.nb_players = nb_players;
    gp.tab_players = (player_t*) malloc (nb_players * sizeof(player_t));
    for(int i = 0;i<nb_players;i++){
        printf("entrez le nom du joueur %d (max 50 car) : ",i+1);
        fgets(gp.tab_players[i].name,sizeof(gp.tab_players[i].name),stdin);
        gp.tab_players[i].col_meeple = tab_colors[i];
        gp.tab_players[i].meeples = NB_MEEPLES;
        gp.tab_players[i].is_ia = false;
    }
    if(nb_players < 5){
        char answer;
        int nb_ia;
        if(nb_players > 1){
        printf("voulez vous inclure une(des) IA dans la partie ? (y/n)");
        scanf("%c",&answer);
        }
        if(answer == 'y' || answer == 'Y' || nb_players == 1){
            printf("entrez le nombre d\'ia que vous voulez (max %d)",5-nb_players);
            while(scanf("%d",&nb_ia) != 1 || nb_ia < 0 || nb_ia + nb_players > 5 || nb_ia + nb_players < 2){
                printf("entrez un nombre d\'ia valides\n");
                printf("entrez le nombre d\'ia que vous voulez (max %d)",5-nb_players);
                while(getchar() != '\n');
            }
            for(int i = nb_players; i < nb_players + nb_ia; i++) {
                int ia_num = i - nb_players + 1;
                char ia_name[16]; 
                snprintf(ia_name, sizeof(ia_name), "IA_%d", ia_num);

                gp.tab_players[i].col_meeple = tab_colors[i];
                gp.tab_players[i].meeples    = NB_MEEPLES;
                gp.tab_players[i].is_ia      = true;
                strcpy(gp.tab_players[i].name, ia_name);
                gp.nb_players++;
            }
        }
    

    }
    return gp;
}


tile_t pick_tile (tiles_pack_t* tp){
    tile_t t = tp->tab_tiles[tp->remaining_tiles - 1];
    tp->remaining_tiles--;
    return t;
}

void place_tile (board_t* board,tile_t t,int x,int y){
    board->grid[x][y] = t;
}

void rotate_tile(tile_t* t, degree_t deg) {
    switch(deg) {
        case ZERO:
            break;
        case NINETY: {
            type_zone_t temp = t->tab_zones[NORTH];
            t->tab_zones[NORTH] = t->tab_zones[WEST];
            t->tab_zones[WEST]  = t->tab_zones[SOUTH];
            t->tab_zones[SOUTH] = t->tab_zones[EAST];
            t->tab_zones[EAST]  = temp;
            break;
        }
        case ONE_HUNDRED_EIGHTY: {
            type_zone_t temp = t->tab_zones[NORTH];
            t->tab_zones[NORTH] = t->tab_zones[SOUTH];
            t->tab_zones[SOUTH] = temp;
            
            temp = t->tab_zones[EAST];
            t->tab_zones[EAST]  = t->tab_zones[WEST];
            t->tab_zones[WEST]  = temp;
            break;
        }
        case TWO_HUNDRED_SEVENTY: {
            type_zone_t temp = t->tab_zones[NORTH];
            t->tab_zones[NORTH] = t->tab_zones[EAST];
            t->tab_zones[EAST]  = t->tab_zones[SOUTH];
            t->tab_zones[SOUTH] = t->tab_zones[WEST];
            t->tab_zones[WEST]  = temp;
            break;
        }
        default:
            break;
    }
}





