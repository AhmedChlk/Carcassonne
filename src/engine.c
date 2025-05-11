#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "board.h"
#include "tile.h"
#include "engine.h"
#include "player.h"

static bool *scored = NULL;

static void reset_scored(void) {
    free(scored);
    scored = NULL;
}


void free_game(game_t* game) {
    free_frontier(&game->frontier);
    free_valid_positions(&game->valid_positions);
    free_board(game->board);
    free(game->players.players);
    reset_scored();

}

board_t* initialize_board(int h, int w) {
    reset_scored();
    board_t* bd = malloc(sizeof *bd);
    if (!bd) { fprintf(stderr, "Erreur alloc board\n"); exit(EXIT_FAILURE); }
    bd->height = h;
    bd->width  = w;
    bd->grid = malloc(h * sizeof *bd->grid);
    if (!bd->grid) { fprintf(stderr, "Erreur alloc rows\n"); exit(EXIT_FAILURE); }
    for (int i = 0; i < h; i++) {
        bd->grid[i] = malloc(w * sizeof *bd->grid[i]);
        if (!bd->grid[i]) { fprintf(stderr, "Erreur alloc row %d\n", i); exit(EXIT_FAILURE); }
    }
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            for (int k = 0; k < 5; k++)
                bd->grid[i][j].zones[k] = NONE;
            bd->grid[i][j].has_meeple = false;
            bd->grid[i][j].meeple_position = NO_DIRECTION;
            bd->grid[i][j].meeple_color = NO_COLOR;
        }
    }
    return bd;
}

void free_board(board_t* bd) {
    for (int i = 0; i < bd->height; i++)
        free(bd->grid[i]);
    free(bd->grid);
    free(bd);
}

tiles_pack_t initialize_tile_pack(const char* fn) {
    char path[256] = "data/";
    strcat(path, fn);
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr, "Erreur open %s\n", path); exit(EXIT_FAILURE); }
    tiles_pack_t tp;
    tp.remaining_tiles = 10;
    char buf[1024];
    int zi, ti = 0;
    while (fgets(buf, sizeof buf, f)) {
        zi = 0;
        char* tok = strtok(buf, ",\n");
        while (tok) {
            if      (!strcmp(tok, "blason"))  tp.tiles[ti].zones[zi++] = SHIELD;
            else if (!strcmp(tok, "route"))   tp.tiles[ti].zones[zi++] = ROAD;
            else if (!strcmp(tok, "pre"))     tp.tiles[ti].zones[zi++] = FIELD;
            else if (!strcmp(tok, "village")) tp.tiles[ti].zones[zi++] = VILLAGE;
            else if (!strcmp(tok, "ville"))   tp.tiles[ti].zones[zi++] = CITY;
            else if (!strcmp(tok, "abbaye"))  tp.tiles[ti].zones[zi++] = ABBEY;
            tok = strtok(NULL, ",\n");
        }
        tp.tiles[ti].has_meeple = false;
        tp.tiles[ti].meeple_position = NO_DIRECTION;
        tp.tiles[ti].meeple_color = NO_COLOR;
        ti++;
    }
    fclose(f);
    return tp;
}


void shuffle_tile_pack(tiles_pack_t* tp) {
    for (int i = 0; i < NB_TILES; i++) {
        int a = rand() % NB_TILES, b;
        do { b = rand() % NB_TILES; } while (a == b);
        tile_t tmp = tp->tiles[a];
        tp->tiles[a] = tp->tiles[b];
        tp->tiles[b] = tmp;
    }
}

game_players_t create_players(){
    game_players_t gp;
    color_t tab_colors[5] = {RED,BLUE,GREEN,YELLOW,WHITE};
    int nb_players;

    printf("À combien de joueurs allez-vous jouer (min 1, max 5) : ");
    while(scanf("%d",&nb_players) != 1  || nb_players < 1 || nb_players > 5){
        printf("Erreur : entrez une valeur valide.\n");
        printf("À combien de joueurs (1–5) : ");
        while(getchar()!='\n');
    }
    while(getchar()!='\n');

    // Allouer pour le maximum dès le départ !
    gp.players = malloc(5 * sizeof(player_t));
    gp.num_players = nb_players;

    // Saisie des joueurs humains
    for(int i = 0; i < nb_players; i++){
        printf("Entrez le nom du joueur %d (max 50 car.) : ", i+1);
        fgets(gp.players[i].name, sizeof(gp.players[i].name), stdin);
        // Supprime le '\n' éventuel de fgets
        gp.players[i].name[strcspn(gp.players[i].name, "\n")] = '\0';
        gp.players[i].meeple_color = tab_colors[i];
        gp.players[i].meeples = NB_MEEPLES;
        gp.players[i].is_ai = false;
        gp.players[i].score = 0;
    }

    // Si on peut ajouter des IA
    if (nb_players < 5) {
        char answer = '\0';
        int nb_ia = 0;
        printf("Voulez-vous inclure des IA ? (y/n) : ");
        scanf(" %c", &answer);
        while(getchar()!='\n');

        if ((answer=='y' || answer=='Y') && nb_players < 5) {
            printf("Entrez le nombre d'IA (0–%d) : ", 5 - nb_players);
            while(scanf("%d", &nb_ia) != 1
                  || nb_ia < 0 
                  || nb_ia > 5 - nb_players) {
                printf("Nombre invalide, retentez (0–%d) : ", 5 - nb_players);
                while(getchar()!='\n');
            }
            while(getchar()!='\n');

            // Ajout des IA dans le même tableau
            for(int i = 0; i < nb_ia; i++){
                int idx = nb_players + i;
                snprintf(gp.players[idx].name,
                         sizeof(gp.players[idx].name),
                         "IA_%d", i+1);
                gp.players[idx].meeple_color = tab_colors[idx];
                gp.players[idx].meeples = NB_MEEPLES;
                gp.players[idx].is_ai = true;
                gp.players[idx].score = 0;
            }
            gp.num_players += nb_ia;
        }
    }

    return gp;
}

tile_t pick_tile(tiles_pack_t* tp) {
    return tp->tiles[--tp->remaining_tiles];
}

void rotate_tile(tile_t* t, degree_t deg) {
    zone_type_t tmp;
    switch (deg) {
        case NINETY:
            tmp = t->zones[NORTH];
            t->zones[NORTH] = t->zones[WEST];
            t->zones[WEST]  = t->zones[SOUTH];
            t->zones[SOUTH] = t->zones[EAST];
            t->zones[EAST]  = tmp;
            break;
        case ONE_HUNDRED_EIGHTY:
            tmp = t->zones[NORTH]; t->zones[NORTH] = t->zones[SOUTH]; t->zones[SOUTH] = tmp;
            tmp = t->zones[EAST];  t->zones[EAST]  = t->zones[WEST];  t->zones[WEST]  = tmp;
            break;
        case TWO_HUNDRED_SEVENTY:
            tmp = t->zones[NORTH];
            t->zones[NORTH] = t->zones[EAST];
            t->zones[EAST]  = t->zones[SOUTH];
            t->zones[SOUTH] = t->zones[WEST];
            t->zones[WEST]  = tmp;
            break;
        default:
            break;
    }
}


bool is_cell_empty(board_t* bd, position_t pos) {
    for (int i = 0; i < 5; i++)
        if (bd->grid[pos.x][pos.y].zones[i] != NONE)
            return false;
    return true;
}

bool is_in_frontier(frontiers_t* f, position_t p) {
    for (int i = 0; i < f->actual; i++)
        if (f->positions[i].x == p.x && f->positions[i].y == p.y)
            return true;
    return false;
}

frontiers_t* allocate_frontier(void) {
    frontiers_t* f = malloc(sizeof *f);
    f->actual   = 0;
    f->capacity = NB_TILES;
    f->positions = malloc(f->capacity * sizeof(position_t));
    return f;
}

void free_frontier(frontiers_t** f) {
    free((*f)->positions);
    free(*f);
    *f = NULL;
}

void add_to_frontier(frontiers_t* f, position_t p) {
    if (!is_in_frontier(f, p))
        f->positions[f->actual++] = p;
}

void remove_from_frontier(frontiers_t* f, position_t p) {
    for (int i = 0; i < f->actual; i++) {
        if (f->positions[i].x == p.x && f->positions[i].y == p.y) {
            f->positions[i] = f->positions[--f->actual];
            return;
        }
    }
}

void update_frontier_after_placement(board_t* bd, frontiers_t* f, position_t p) {
    position_t dirs[4] = {{-1,0},{0,1},{1,0},{0,-1}};
    for (int k = 0; k < 4; k++) {
        position_t q = { p.x + dirs[k].x, p.y + dirs[k].y };
        if (q.x >= 0 && q.x < bd->height && q.y >= 0 && q.y < bd->width && is_cell_empty(bd, q))
            add_to_frontier(f, q);
    }
}

void place_meeple(board_t* bd, game_players_t* gp, direction_t d, position_t p, color_t c) {
    bd->grid[p.x][p.y].has_meeple     = true;
    bd->grid[p.x][p.y].meeple_position = d;
    bd->grid[p.x][p.y].meeple_color    = c;
    for (int i = 0; i < gp->num_players; i++) {
        if (gp->players[i].meeple_color == c) {
            gp->players[i].meeples--;
            break;
        }
    }
}

void remove_meeple(board_t* bd, game_players_t* gp, position_t p) {
    color_t col = bd->grid[p.x][p.y].meeple_color;
    bd->grid[p.x][p.y].has_meeple     = false;
    bd->grid[p.x][p.y].meeple_position = NO_DIRECTION;
    bd->grid[p.x][p.y].meeple_color    = NO_COLOR;
    for (int i = 0; i < gp->num_players; i++) {
        if (gp->players[i].meeple_color == col) {
            gp->players[i].meeples++;
            break;
        }
    }
}

valid_positions_t* allocate_valid_positions(void) {
    valid_positions_t* v = malloc(sizeof *v);
    v->actual   = 0;
    v->capacity = NB_TILES;
    v->valid_positions = malloc(v->capacity * sizeof(position_t));
    return v;
}

void free_valid_positions(valid_positions_t** v) {
    free((*v)->valid_positions);
    free(*v);
    *v = NULL;
}

void reset_valid_positions(valid_positions_t* v) {
    v->actual = 0;
}

void add_to_valid_positions(valid_positions_t* v, position_t p) {
    if (v->actual == v->capacity) {
        v->capacity *= 2;
        v->valid_positions = realloc(v->valid_positions, v->capacity * sizeof(position_t));
    }
    v->valid_positions[v->actual++] = p;
}

bool check_matching_zones(board_t* bd, tile_t t, position_t p) {
    position_t dirs[4] = {{-1,0},{0,1},{1,0},{0,-1}};
    direction_t opp[4] = { SOUTH, WEST, NORTH, EAST };
    for (int k = 0; k < 4; k++) {
        int nx = p.x + dirs[k].x;
        int ny = p.y + dirs[k].y;
        if (nx>=0 && nx<bd->height && ny>=0 && ny<bd->width && bd->grid[nx][ny].zones[opp[k]]!=NONE) {
            zone_type_t z = bd->grid[nx][ny].zones[opp[k]];
            if (t.zones[k]!=z && !((t.zones[k]==SHIELD&&z==CITY)||(t.zones[k]==CITY&&z==SHIELD)))
                return false;
        }
    }
    return true;
}


void get_placement_tile(board_t* board,
    frontiers_t* frontier,
    tile_t tile,
    degree_t rot,
    valid_positions_t* v)
{
// tourne une copie selon rot
tile_t tmp = tile;
for (int r = 0; r < (int)rot; r++) {
rotate_tile(&tmp, NINETY);
}
reset_valid_positions(v);
// teste uniquement la rotation courante
for (int i = 0; i < frontier->actual; i++) {
position_t p = frontier->positions[i];
if (check_matching_zones(board, tmp, p)) {
add_to_valid_positions(v, p);
}
}
}

game_t init_game(int H, int W, const char* fn) {
    game_t G;
    G.board           = initialize_board(H, W);
    G.frontier        = allocate_frontier();
    G.valid_positions = allocate_valid_positions();
    G.tile_pack       = initialize_tile_pack(fn);
    tile_t st         = pick_tile(&G.tile_pack);
    position_t c      = { H/2, W/2 };
    place_tile(G.board, G.frontier, st, c);
    shuffle_tile_pack(&G.tile_pack);
    G.players         = create_players();
    return G;
}

bool feature_matches(zone_type_t a, zone_type_t b) {
    return a==b || (a==CITY && b==SHIELD) || (a==SHIELD && b==CITY);
}

bool dfs_has_meeple(board_t* bd,int x,int y,direction_t z,zone_type_t tgt,bool* vs) {
    int idx = (x*bd->width + y)*5 + z;
    if(vs[idx]) return false;
    vs[idx] = true;
    tile_t* tl = &bd->grid[x][y];
    if(tl->has_meeple && tl->meeple_position == z) return true;
    if(z==CENTER || feature_matches(tl->zones[CENTER], tgt)) {
        for (direction_t i = 0; i < CENTER; i++) {
            if (i!=z && feature_matches(tl->zones[i], tgt)) {
                if (dfs_has_meeple(bd,x,y,i,tgt,vs)) return true;
            }
        }
    }
    if(z < CENTER) {
        static const int DX[4]={-1,0,1,0}, DY[4]={0,1,0,-1};
        static const direction_t OPP[4]={SOUTH,WEST,NORTH,EAST};
        int nx=x+DX[z], ny=y+DY[z];
        if(nx>=0&&nx<bd->height&&ny>=0&&ny<bd->width
           && feature_matches(bd->grid[nx][ny].zones[OPP[z]], tgt)) {
            if(dfs_has_meeple(bd,nx,ny,OPP[z],tgt,vs)) return true;
        }
    }
    return false;
}

meeple_options_t get_placement_meeple(board_t* bd, position_t p) {
    meeple_options_t opt = {{false,false,false,false,false}};
    tile_t* tl = &bd->grid[p.x][p.y];
    if(tl->has_meeple) return opt;
    int ts = bd->height * bd->width * 5;
    bool* vs = calloc(ts, sizeof(bool));
    for(int z=0; z<5; z++){
        zone_type_t tp = tl->zones[z];
        if(tp==NONE) continue;
        if(tp==ABBEY) opt.available[z] = true;
        else if(tp==ROAD||tp==CITY||tp==SHIELD){
            memset(vs,0,ts);
            if(!dfs_has_meeple(bd,p.x,p.y,(direction_t)z,tp,vs))
                opt.available[z] = true;
        }
    }
    free(vs);
    return opt;
}

void place_tile(board_t* bd, frontiers_t* f, tile_t t, position_t p) {
    bd->grid[p.x][p.y] = t;
    if(f) {
        update_frontier_after_placement(bd, f, p);
        if(is_in_frontier(f, p)) remove_from_frontier(f, p);
    }
}

static inline bool is_connected_zone(zone_type_t zone, zone_type_t feature) {
    return zone == feature || (feature == CITY && zone == SHIELD);
}

static bool has_tile(board_t* bd, int x, int y) {
    for (int d = 0; d < 5; d++) {
        if (bd->grid[x][y].zones[d] != NONE)
            return true;
    }
    return false;
}
structure_score_t bfs_structure(board_t* board,game_players_t* players,int start_x,int start_y,zone_type_t feature,bool* visited) {
    int H = board->height, W = board->width, N = H * W;
    const int DX[4] = {-1, 0, 1, 0}, DY[4] = {0, 1, 0, -1}, OPP[4] = {SOUTH, WEST, NORTH, EAST};

    structure_score_t result = {0, 0, calloc(players->num_players, sizeof(int))};
    int* queue = malloc(N * sizeof(int));
    int head = 0, tail = 0;

    int start_id = start_x * W + start_y;
    queue[tail++] = start_id;
    visited[start_id] = true;

    while (head < tail) {
        int id = queue[head++], x = id / W, y = id % W;
        tile_t* tile = &board->grid[x][y];

        bool valid = false;
        for (int d = 0; d < 5 && !valid; d++)
            if (is_connected_zone(tile->zones[d], feature)) valid = true;
        if (!valid) continue;

        result.tile_count++;
        if (feature == CITY)
            for (int d = 0; d < 5; d++)
                if (tile->zones[d] == SHIELD) {
                    result.shield_count++;
                    break;
                }

        if (tile->has_meeple) {
            for (int p = 0; p < players->num_players; p++) {
                if (players->players[p].meeple_color == tile->meeple_color)
                    result.meeple_count_per_player[p]++;
            }
        }

        for (int d = 0; d < 4; d++) {
            if (!is_connected_zone(tile->zones[d], feature)) continue;
            int nx = x + DX[d], ny = y + DY[d];
            if (nx < 0 || ny < 0 || nx >= H || ny >= W) continue;
            tile_t* neighbor = &board->grid[nx][ny];
            if (!is_connected_zone(neighbor->zones[OPP[d]], feature)) continue;
            int nid = nx * W + ny;
            if (!visited[nid]) {
                visited[nid] = true;
                queue[tail++] = nid;
            }
        }
    }

    free(queue);
    return result;
}

void score_incomplete_structures(game_t* game) {
    board_t* bd = game->board;
    game_players_t* gp = &game->players;
    int H = bd->height, W = bd->width, N = H * W;

    bool* scored = calloc(N * 5, sizeof(bool));

    for (int x = 0; x < H; x++) {
        for (int y = 0; y < W; y++) {
            int i = (x * W + y) * 5 + CENTER;
            if (!scored[i] && bd->grid[x][y].zones[CENTER] == ABBEY) {
                int adj = 0;
                for (int dx = -1; dx <= 1; dx++)
                    for (int dy = -1; dy <= 1; dy++)
                        if ((dx || dy) && x + dx >= 0 && x + dx < H && y + dy >= 0 && y + dy < W
                            && has_tile(bd, x + dx, y + dy))
                            adj++;

                int pts = adj;
                tile_t* abb = &bd->grid[x][y];
                if (abb->has_meeple && abb->meeple_position == CENTER) {
                    for (int p = 0; p < gp->num_players; p++) {
                        if (gp->players[p].meeple_color == abb->meeple_color) {
                            gp->players[p].score += pts;
                            printf("Joueur %s marque %d pts (abbaye incomplète)\n", gp->players[p].name, pts);
                            break;
                        }
                    }
                    remove_meeple(bd, gp, (position_t){x, y});
                }
                scored[i] = true;
            }
        }
    }

    bool* visited_city = calloc(N, sizeof(bool));
    bool* visited_road = calloc(N, sizeof(bool));

    for (int kind = 0; kind < 2; kind++) {
        zone_type_t feat = (kind == 0 ? CITY : ROAD);
        bool* visited = (kind == 0 ? visited_city : visited_road);
        const char* label = (kind == 0 ? "ville" : "route");

        for (int id = 0; id < N; id++) {
            if (visited[id]) continue;
            int x = id / W, y = id % W;
            tile_t* t = &bd->grid[x][y];

            bool has_feat = false;
            for (int d = 0; d < 5 && !has_feat; d++)
                if (is_connected_zone(t->zones[d], feat)) has_feat = true;
            if (!has_feat) continue;

            structure_score_t s = bfs_structure(bd, gp, x, y, feat, visited);
            int pts = s.tile_count + s.shield_count;

            int best = 0;
            for (int p = 0; p < gp->num_players; p++)
                if (s.meeple_count_per_player[p] > best) best = s.meeple_count_per_player[p];

            if (best > 0) {
                for (int p = 0; p < gp->num_players; p++) {
                    if (s.meeple_count_per_player[p] == best) {
                        gp->players[p].score += pts;
                        printf("Joueur %s marque %d pts (%s incomplète)\n", gp->players[p].name, pts, label);
                    }
                }
                for (int i = 0; i < N; i++) {
                    if (visited[i] && bd->grid[i / W][i % W].has_meeple)
                        remove_meeple(bd, gp, (position_t){i / W, i % W});
                }
            }
            free(s.meeple_count_per_player);
        }
    }

    free(visited_city);
    free(visited_road);
    free(scored);
}

void score_completed_structures(game_t* game, position_t last_pos) {
    board_t* bd = game->board;
    game_players_t* gp = &game->players;
    int H = bd->height, W = bd->width, total_z = H * W * 5;

    if (!scored) {
        scored = calloc(total_z, sizeof(bool));
        if (!scored) { perror("calloc"); exit(EXIT_FAILURE); }
    }

    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int ax = last_pos.x + dx, ay = last_pos.y + dy;
            if (ax < 0 || ax >= H || ay < 0 || ay >= W) continue;

            int idx = (ax * W + ay) * 5 + CENTER;
            if (scored[idx] || bd->grid[ax][ay].zones[CENTER] != ABBEY) continue;

            bool complete = true;
            for (int i = -1; i <= 1 && complete; i++)
                for (int j = -1; j <= 1; j++) {
                    int xx = ax + i, yy = ay + j;
                    if (xx < 0 || xx >= H || yy < 0 || yy >= W || !has_tile(bd, xx, yy))
                        complete = false;
                }

            if (!complete) continue;
            scored[idx] = true;

            tile_t* abb = &bd->grid[ax][ay];
            if (abb->has_meeple && abb->meeple_position == CENTER) {
                for (int p = 0; p < gp->num_players; p++) {
                    if (gp->players[p].meeple_color == abb->meeple_color) {
                        gp->players[p].score += 9;
                        printf("Joueur %s marque 9 pts (abbaye[%d,%d])\n",
                               gp->players[p].name, ax, ay);
                        break;
                    }
                }
                remove_meeple(bd, gp, (position_t){ax, ay});
            }
        }
    }

    const int DX[4] = {-1, 0, 1, 0}, DY[4] = {0, 1, 0, -1}, OPP[4] = {SOUTH, WEST, NORTH, EAST};

    for (int d = 0; d < 4; d++) {
        zone_type_t feature = bd->grid[last_pos.x][last_pos.y].zones[d];
// Si SHIELD, on le traite comme CITY
if (feature == SHIELD) feature = CITY;
if (feature != ROAD && feature != CITY) continue;
        int start_idx = (last_pos.x * W + last_pos.y) * 5 + d;
        if (scored[start_idx]) continue;

        bool* zvis = calloc(total_z, sizeof(bool));
        bool* tvis = calloc(H * W, sizeof(bool));
        int* queue = malloc(total_z * sizeof(int));
        int* mcount = calloc(gp->num_players, sizeof(int));
        int qh = 0, qt = 0, tcount = 0, scount = 0;
        bool closed = true;

        zvis[start_idx] = true;
        queue[qt++] = start_idx;

        while (qh < qt) {
            int idx = queue[qh++], zid = idx % 5, tid = idx / 5;
            int x = tid / W, y = tid % W;
            tile_t* tile = &bd->grid[x][y];

            if (!tvis[tid]) {
                tvis[tid] = true;
                tcount++;
                if (feature == CITY)
                    for (int i = 0; i < 5; i++)
                        if (tile->zones[i] == SHIELD)
                            scount++;
            }

            if (tile->has_meeple && tile->meeple_position == zid) {
                for (int p = 0; p < gp->num_players; p++) {
                    if (gp->players[p].meeple_color == tile->meeple_color)
                        mcount[p]++;
                }
            }

            for (int dz = 0; dz < 5; dz++) {
                int didx = tid * 5 + dz;
                if (dz != zid && !zvis[didx]
                    && is_connected_zone(tile->zones[dz], feature)
                    && (zid == CENTER || dz == CENTER || tile->zones[CENTER] == feature)) {
                    zvis[didx] = true;
                    queue[qt++] = didx;
                }
            }

            if (zid < 4) {
                int nx = x + DX[zid], ny = y + DY[zid];
                if (nx < 0 || nx >= H || ny < 0 || ny >= W ||
                    !is_connected_zone(bd->grid[nx][ny].zones[OPP[zid]], feature)) {
                    closed = false;
                } else {
                    int nidx = (nx * W + ny) * 5 + OPP[zid];
                    if (!zvis[nidx]) {
                        zvis[nidx] = true;
                        queue[qt++] = nidx;
                    }
                }
            }
        }

        if (closed) {
            for (int i = 0; i < total_z; i++) if (zvis[i]) scored[i] = true;

            int best = 0;
            for (int p = 0; p < gp->num_players; p++)
                if (mcount[p] > best) best = mcount[p];

            if (best > 0) {
                int pts = feature == ROAD ? tcount : (tcount * 2 + scount * 2);
                const char* name = feature == ROAD ? "route" : "ville";

                for (int p = 0; p < gp->num_players; p++) {
                    if (mcount[p] == best) {
                        gp->players[p].score += pts;
                        printf("Joueur %s marque %d pts (%s)\n",
                               gp->players[p].name, pts, name);
                    }
                }

                for (int tid = 0; tid < H * W; tid++) {
                    if (tvis[tid] && bd->grid[tid / W][tid % W].has_meeple)
                        remove_meeple(bd, gp, (position_t){tid / W, tid % W});
                }
            }
        }

        free(zvis);
        free(tvis);
        free(queue);
        free(mcount);
    }
}