#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "board.h"
#include "tile.h"
#include "player.h"

board_t* initialize_board(int height, int width) {
    board_t* board = (board_t*) malloc(sizeof(board_t));
    if (board == NULL) {
        fprintf(stderr, "Error allocating memory for board\n");
        exit(EXIT_FAILURE);
    }
    board->height = height;
    board->width = width;
    board->grid = (tile_t**) malloc(height * sizeof(tile_t*));
    for (int i = 0; i < height; i++) {
        board->grid[i] = (tile_t*) malloc(width * sizeof(tile_t));
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int k = 0; k < 5; k++) {
                board->grid[i][j].zones[k] = NONE;
            }
            board->grid[i][j].has_meeple = false;
            board->grid[i][j].meeple_position = NO_DIRECTION;
            board->grid[i][j].meeple_color = NO_COLOR;
        }
    }
    return board;
}

void free_board(board_t* board) {
    for (int i = 0; i < board->height; i++) {
        free(board->grid[i]);
    }
    free(board->grid);
    free(board);
}

tiles_pack_t initialize_tile_pack(const char* filename) {
    char path[256] = "data/";
    strcat(path, filename);
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(EXIT_FAILURE);
    }
    int buffer_size = 1024;
    char buffer[1024];
    int zone_index, tile_index = 0;
    tiles_pack_t tile_pack;
    tile_pack.remaining_tiles = NB_TILES;
    while (fgets(buffer, buffer_size, file) != NULL) {
        zone_index = 0;
        char* token = strtok(buffer, ",\n");
        while (token != NULL) {
            if (strcmp(token, "blason") == 0)
                tile_pack.tiles[tile_index].zones[zone_index++] = SHIELD;
            else if (strcmp(token, "route") == 0)
                tile_pack.tiles[tile_index].zones[zone_index++] = ROAD;
            else if (strcmp(token, "pre") == 0)
                tile_pack.tiles[tile_index].zones[zone_index++] = FIELD;
            else if (strcmp(token, "village") == 0)
                tile_pack.tiles[tile_index].zones[zone_index++] = VILLAGE;
            else if (strcmp(token, "ville") == 0)
                tile_pack.tiles[tile_index].zones[zone_index++] = CITY;
            else if (strcmp(token, "abbaye") == 0)
                tile_pack.tiles[tile_index].zones[zone_index++] = ABBEY;
            token = strtok(NULL, ",\n");
        }
        tile_pack.tiles[tile_index].has_meeple = false;
        tile_pack.tiles[tile_index].meeple_position = NO_DIRECTION;
        tile_pack.tiles[tile_index].meeple_color = NO_COLOR;
        tile_index++;
    }
    fclose(file);
    return tile_pack;
}

void shuffle_tile_pack(tiles_pack_t* tile_pack) {
    tile_t temp;
    int index1, index2;
    for (int i = 0; i < NB_TILES; i++) {
        do {
            index1 = rand() % NB_TILES;
            index2 = rand() % NB_TILES;
        } while (index1 == index2);
        temp = tile_pack->tiles[index1];
        tile_pack->tiles[index1] = tile_pack->tiles[index2];
        tile_pack->tiles[index2] = temp;
    }
}

game_players_t create_players() {
    game_players_t game_players;
    color_t available_colors[5] = {RED, BLUE, GREEN, YELLOW, WHITE};
    int num_players;
    printf("How many players will participate (min 1, max 5): ");
    while (scanf("%d", &num_players) != 1 || num_players < 1 || num_players > 5) {
        printf("Error: Please enter a valid value\n");
        printf("How many players will participate (min 1, max 5): ");
        while (getchar() != '\n');
    }
    while (getchar() != '\n');
    game_players.num_players = num_players;
    game_players.players = (player_t*) malloc(num_players * sizeof(player_t));
    for (int i = 0; i < num_players; i++) {
        printf("Enter the name of player %d (max 50 characters): ", i + 1);
        fgets(game_players.players[i].name, sizeof(game_players.players[i].name), stdin);
        game_players.players[i].name[strcspn(game_players.players[i].name, "\n")] = '\0';
        game_players.players[i].meeple_color = available_colors[i];
        game_players.players[i].meeples = NB_MEEPLES;
        game_players.players[i].is_ai = false;
    }
    if (num_players < 5) {
        char answer;
        int num_ai;
        if (num_players > 1) {
            printf("Do you want to include AI players in the game? (y/n): ");
            scanf(" %c", &answer);
            while (getchar() != '\n');
        }
        if (num_players == 1 || answer == 'y' || answer == 'Y') {
            printf("Enter the number of AI players you want (max %d): ", 5 - num_players);
            while (scanf("%d", &num_ai) != 1 || num_ai < 0 || num_ai + num_players > 5 || num_ai + num_players < 2) {
                printf("Please enter a valid number of AI players\n");
                printf("Enter the number of AI players you want (max %d): ", 5 - num_players);
                while (getchar() != '\n');
            }
            game_players.players = (player_t*) realloc(game_players.players, (num_players + num_ai) * sizeof(player_t));
            for (int i = num_players; i < num_players + num_ai; i++) {
                int ai_num = i - num_players + 1;
                char ai_name[16];
                snprintf(ai_name, sizeof(ai_name), "AI_%d", ai_num);
                game_players.players[i].meeple_color = available_colors[i];
                game_players.players[i].meeples = NB_MEEPLES;
                game_players.players[i].is_ai = true;
                strcpy(game_players.players[i].name, ai_name);
                game_players.num_players++;
            }
        }
    }
    return game_players;
}

tile_t pick_tile(tiles_pack_t* tile_pack) {
    tile_t t = tile_pack->tiles[tile_pack->remaining_tiles - 1];
    tile_pack->remaining_tiles--;
    return t;
}

void rotate_tile(tile_t* t, degree_t deg) {
    switch (deg) {
        case ZERO:
            break;
        case NINETY: {
            zone_type_t temp = t->zones[NORTH];
            t->zones[NORTH] = t->zones[WEST];
            t->zones[WEST]  = t->zones[SOUTH];
            t->zones[SOUTH] = t->zones[EAST];
            t->zones[EAST]  = temp;
            break;
        }
        case ONE_HUNDRED_EIGHTY: {
            zone_type_t temp = t->zones[NORTH];
            t->zones[NORTH] = t->zones[SOUTH];
            t->zones[SOUTH] = temp;
            
            temp = t->zones[EAST];
            t->zones[EAST]  = t->zones[WEST];
            t->zones[WEST]  = temp;
            break;
        }
        case TWO_HUNDRED_SEVENTY: {
            zone_type_t temp = t->zones[NORTH];
            t->zones[NORTH] = t->zones[EAST];
            t->zones[EAST]  = t->zones[SOUTH];
            t->zones[SOUTH] = t->zones[WEST];
            t->zones[WEST]  = temp;
            break;
        }
        default:
            break;
    }
}

bool is_cell_empty(board_t* board, position_t pos) {
    for (int i = 0; i < 5; i++) {
        if (board->grid[pos.x][pos.y].zones[i] != NONE) {
            return false;
        }
    }
    return true;
}

bool is_in_frontier(frontiers_t* frontier, position_t pos) {
    for (int i = 0; i < frontier->actual; i++) {
        if (frontier->positions[i].x == pos.x && frontier->positions[i].y == pos.y) {
            return true;
        }
    }
    return false;
}

frontiers_t* allocate_frontier() {
    frontiers_t* frontier = (frontiers_t*) malloc(sizeof(frontiers_t));
    frontier->actual = 0;
    frontier->capacity = NB_TILES;
    frontier->positions = (position_t*) malloc(frontier->capacity * sizeof(position_t));
    for (int i = 0; i < frontier->capacity; i++) {
        frontier->positions[i].x = -1;
        frontier->positions[i].y = -1;
    }
    return frontier;
}

void free_frontier(frontiers_t** frontier) {
    free((*frontier)->positions);
    free(*frontier);
    *frontier = NULL;
}

void add_to_frontier(frontiers_t* frontier, position_t pos) {
    if (!is_in_frontier(frontier, pos)) {
        if (frontier->actual == frontier->capacity - 1) {
            frontier->capacity *= 2;
            frontier->positions = realloc(frontier->positions, frontier->capacity * sizeof(position_t));
        }
        frontier->positions[frontier->actual++] = pos;
    }
}

void remove_from_frontier(frontiers_t* frontier, position_t pos) {
    int i = 0;
    while (i < frontier->actual && (frontier->positions[i].x != pos.x || frontier->positions[i].y != pos.y)) {
        i++;
    }
    if (i >= frontier->actual) {
        fprintf(stderr, "Error: Position not found in frontier when trying to remove\n");
        exit(EXIT_FAILURE);
    }
    for (int j = i; j < frontier->actual - 1; j++) {
        frontier->positions[j] = frontier->positions[j + 1];
    }
    frontier->actual--;
}

void update_frontier_after_placement(board_t* board, frontiers_t* frontier, position_t pos) {
    position_t delta[4] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };
    for (int k = 0; k < 4; k++) {
        int nx = pos.x + delta[k].x;
        int ny = pos.y + delta[k].y;
        if (nx >= 0 && nx < board->height && ny >= 0 && ny < board->width) {
            if (is_cell_empty(board, (position_t){nx, ny}))
                add_to_frontier(frontier, (position_t){nx, ny});
        }
    }
}

void place_tile(board_t* board, frontiers_t* frontier, tile_t tile, position_t pos) {
    board->grid[pos.x][pos.y] = tile;
    update_frontier_after_placement(board, frontier, pos);
    if (is_in_frontier(frontier, pos)) {
        remove_from_frontier(frontier, pos);
    }
}

// Meeple functions 

void place_meeple(board_t* board, game_players_t* p, direction_t d, position_t pos, color_t c)
{
    /*Place the meeple in tile (pos(x, y)) with the color (c) in direction(d) for the current player in players(p)*/

    board->grid[pos.x][pos.y].meeple_position = d;
    board->grid[pos.x][pos.y].meeple_color = c;
    board->grid[pos.x][pos.y].has_meeple = true;

    for (int i = 0; i < p->num_players; i++)
        if (p->players[i].meeple_color == c) p->players[i].meeples--;
    
}

void remove_meeple(board_t* board, game_players_t* p, direction_t d, position_t pos, color_t c)
{
    /*Remove the meeple in tile (pos(x, y)) with the color (c) in direction(d) for the current player in players(p)*/

    board->grid[pos.x][pos.y].meeple_position = NO_DIRECTION;
    board->grid[pos.x][pos.y].meeple_color = NO_COLOR;
    board->grid[pos.x][pos.y].has_meeple = false;

    for (int i = 0; i < p->num_players; i++)
        if (p->players[i].meeple_color == c) p->players[i].meeples++;
    
}
valid_positions_t* allocate_valid_positions(){
    valid_positions_t* v = (valid_positions_t*) malloc (sizeof(valid_positions_t));
    v->actual = 0;
    v->capacity = NB_TILES;
    v->valid_positions = (position_t*) malloc(v->capacity * sizeof(position_t));
    for (int i = 0; i < v->capacity; i++) {
        v->valid_positions[i].x = -1;
        v->valid_positions[i].y = -1;
    }
    return v;
}
void free_valid_positions (valid_positions_t** v){
    free((*v)->valid_positions);
    free(*v);
    *v = NULL;
}

void reset_valid_positions(valid_positions_t* v){
    for(int i=0;i<v->capacity;i++){
        v->valid_positions[i].x = -1;
        v->valid_positions[i].y = -1;
    }
    v->actual = 0;
}

void add_to_valid_positions (valid_positions_t* v,position_t pos){
    if (v->actual == v->capacity - 1) {
            v->capacity *= 2;
            v->valid_positions = realloc(v->valid_positions, v->capacity * sizeof(position_t));
        }
        v->valid_positions[v->actual++] = pos;
        //printf("ajout de (%d,%d) dans le tableau de positions\n",pos.x,pos.y);
}

bool check_matching_zones(board_t* board,tile_t tile,position_t pos){
    position_t delta[4] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };
    direction_t d[4] = {SOUTH,WEST,NORTH,EAST};
    for (int k = 0; k < 4; k++) {
        int nx = pos.x + delta[k].x;
        int ny = pos.y + delta[k].y;
        if (nx >= 0 && nx < board->height && ny >= 0 && ny < board->width) {
            if(board->grid[nx][ny].zones[d[k]] != NONE){
                if (tile.zones[k] != board->grid[nx][ny].zones[d[k]])
                    return false;
            }
        }
    }
    return true;
}
bool is_in_valid_positions(valid_positions_t* v,position_t p){
    for(int i=0;i<v->actual;i++){
        if(v->valid_positions[i].x == p.x && v->valid_positions[i].y == p.y ){
            return true;
        }
    }
    return false;
}
void get_placement_tile (board_t* b,frontiers_t* frontier,tile_t tile,valid_positions_t* valid_positions){
    reset_valid_positions(valid_positions);
    for(int i=0;i<frontier->actual;i++){
        position_t p = {frontier->positions[i].x,frontier->positions[i].y};
        //printf("test de (%d,%d)\n",p.x,p.y);
        for(int j=0;j<4;j++){
            if(check_matching_zones(b,tile,p)){
                if(!is_in_valid_positions(valid_positions,p)){
                     add_to_valid_positions(valid_positions,p);
                }
            }
            rotate_tile(&tile,NINETY);
        }
    }
}

 
void expand_board(board_t* board) {
    int new_width = board->width * 2;
    int new_height = board->height * 2;
    
    // Nouvelle grille
    tile_t** new_grid = (tile_t**)malloc(new_height * sizeof(tile_t*));
    for (int i = 0; i < new_height; i++) {
        new_grid[i] = (tile_t*)calloc(new_width, sizeof(tile_t));
    }

    // Copier old grid
    int offset_x = board->width / 2;
    int offset_y = board->height / 2;
    
    for (int i = 0; i < board->height; i++) {
        for (int j = 0; j < board->width; j++) {
            new_grid[i + offset_y][j + offset_x] = board->grid[i][j];
        }
    }

    // free  old grid 
    for (int i = 0; i < board->height; i++) {
        free(board->grid[i]);
    }
    free(board->grid);

    // update new dimension
    board->grid = new_grid;
    board->width = new_width;
    board->height = new_height;
}
