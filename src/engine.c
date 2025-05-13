#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "board.h"
#include "tile.h"
#include "engine.h"
#include "player.h"

/* =========== VARIABLES GLOBALES =========== */
static bool *scored = NULL;

/* =========== GESTION MÉMOIRE SCORING =========== */
static void reset_scored(void) {
    free(scored);
    scored = NULL;
}

/* =========== CRÉATION ET DESTRUCTION DU PLATEAU =========== */
board_t* create_board(int height, int width) {
    reset_scored();
    board_t* board = malloc(sizeof *board);
    if (!board) {
        fprintf(stderr, "Erreur : allocation de la structure board\n");
        exit(EXIT_FAILURE);
    }
    board->height = height;
    board->width = width;
    board->grid = malloc(height * sizeof *board->grid);
    if (!board->grid) {
        fprintf(stderr, "Erreur : allocation des lignes du board\n");
        exit(EXIT_FAILURE);
    }
    for (int row = 0; row < height; row++) {
        board->grid[row] = malloc(width * sizeof *board->grid[row]);
        if (!board->grid[row]) {
            fprintf(stderr, "Erreur : allocation de la ligne %d\n", row);
            exit(EXIT_FAILURE);
        }
    }
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            tile_t* tile = &board->grid[row][col];
            for (int z = 0; z < 5; z++) tile->zones[z] = NONE;
            tile->has_meeple = false;
            tile->meeple_position = NO_DIRECTION;
            tile->meeple_color = NO_COLOR;
        }
    }
    return board;
}

void destroy_board(board_t* board) {
    for (int row = 0; row < board->height; row++) {
        free(board->grid[row]);
    }
    free(board->grid);
    free(board);
}

/* =========== GESTION DES TUILES =========== */
tiles_pack_t initialize_tile_pack(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", filename);
        exit(EXIT_FAILURE);
    }
    tiles_pack_t pack = {0};
    char line[1024];
    int tile_index = 0;
    while (fgets(line, sizeof line, file) && tile_index < NB_TILES) {
        int zone_index = 0;
        char* token = strtok(line, ",\n");
        while (token && zone_index < 5) {
            zone_type_t zone = NONE;
            if      (strcmp(token, "blason")  == 0) zone = SHIELD;
            else if (strcmp(token, "route")   == 0) zone = ROAD;
            else if (strcmp(token, "pre")     == 0) zone = FIELD;
            else if (strcmp(token, "village") == 0) zone = VILLAGE;
            else if (strcmp(token, "ville")   == 0) zone = CITY;
            else if (strcmp(token, "abbaye")  == 0) zone = ABBEY;
            pack.tiles[tile_index].zones[zone_index++] = zone;
            token = strtok(NULL, ",\n");
        }
        pack.tiles[tile_index].has_meeple = false;
        pack.tiles[tile_index].meeple_position = NO_DIRECTION;
        pack.tiles[tile_index].meeple_color = NO_COLOR;
        tile_index++;
    }
    pack.remaining_tiles = tile_index;
    fclose(file);
    return pack;
}

void shuffle_tile_pack(tiles_pack_t* pack) {
    for (int i = pack->remaining_tiles - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        tile_t tmp = pack->tiles[i];
        pack->tiles[i] = pack->tiles[j];
        pack->tiles[j] = tmp;
    }
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

bool is_cell_empty(board_t* board, position_t pos) {
    for (int i = 0; i < 5; i++)
        if (board->grid[pos.x][pos.y].zones[i] != NONE)
            return false;
    return true;
}

/* =========== GESTION DES JOUEURS =========== */
game_players_t create_players() {
    game_players_t gp = {0};
    color_t colors[5] = {RED, BLUE, GREEN, YELLOW, WHITE};
    int human_count = 0, ai_count = 0;

    // Saisie des joueurs humains
    do {
        printf("Combien de joueurs humains ? (1 à 5) : ");
        if (scanf("%d", &human_count) != 1 || human_count < 1 || human_count > 5) {
            printf("❌ Saisie invalide. Veuillez entrer un nombre entre 1 et 5.\n");
            while (getchar() != '\n');
            human_count = 0;
        } else {
            while (getchar() != '\n');
        }
    } while (human_count == 0);

    if (human_count == 1) {
        printf("ℹ️  Vous êtes seul(e), au moins une IA est requise pour jouer.\n");
        do {
            printf("Combien d'IA souhaitez-vous ajouter ? (1 à 4) : ");
            if (scanf("%d", &ai_count) != 1 || ai_count < 1 || ai_count > 4) {
                printf("❌ Saisie invalide. Entrez un nombre entre 1 et 4.\n");
                while (getchar() != '\n');
                ai_count = 0;
            } else {
                while (getchar() != '\n');
            }
        } while (ai_count == 0);
    } else if (human_count < 5) {
        char answer = '\0';
        printf("Souhaitez-vous inclure des IA ? (y/n) : ");
        scanf(" %c", &answer);
        while (getchar() != '\n');

        if (answer == 'y' || answer == 'Y') {
            do {
                printf("Combien d'IA voulez-vous ajouter ? (0 à %d) : ", 5 - human_count);
                if (scanf("%d", &ai_count) != 1 || ai_count < 0 || ai_count > 5 - human_count) {
                    printf("❌ Nombre invalide. Entrez une valeur entre 0 et %d.\n", 5 - human_count);
                    while (getchar() != '\n');
                    ai_count = -1;
                } else {
                    while (getchar() != '\n');
                }
            } while (ai_count < 0);
        }
    }

    int total_players = human_count + ai_count;
    gp.players = malloc(total_players * sizeof(player_t));
    gp.num_players = total_players;

    for (int i = 0; i < human_count; i++) {
        printf("Entrez le nom du joueur %d : ", i + 1);
        fgets(gp.players[i].name, sizeof(gp.players[i].name), stdin);
        gp.players[i].name[strcspn(gp.players[i].name, "\n")] = '\0';
        gp.players[i].meeple_color = colors[i];
        gp.players[i].meeples = NB_MEEPLES;
        gp.players[i].is_ai = false;
        gp.players[i].score = 0;
    }

    for (int i = 0; i < ai_count; i++) {
        int idx = human_count + i;
        snprintf(gp.players[idx].name, sizeof(gp.players[idx].name), "IA_%d", i + 1);
        gp.players[idx].meeple_color = colors[idx];
        gp.players[idx].meeples = NB_MEEPLES;
        gp.players[idx].is_ai = true;
        gp.players[idx].score = 0;
    }

    return gp;
}

/* =========== GESTION DES MEEPLES =========== */
void place_meeple(board_t* board, game_players_t* players, direction_t dir, position_t pos, color_t color) {
    tile_t* tile = &board->grid[pos.x][pos.y];
    tile->has_meeple      = true;
    tile->meeple_position = dir;
    tile->meeple_color    = color;
    for (int i = 0; i < players->num_players; i++) {
        if (players->players[i].meeple_color == color) {
            if (players->players[i].meeples > 0) {
                players->players[i].meeples--;
            } else {
                fprintf(stderr, "⚠️ Le joueur %s n’a plus de meeples !\n", players->players[i].name);
            }
            break;
        }
    }
}

void remove_meeple(board_t* board, game_players_t* players, position_t pos) {
    tile_t* tile = &board->grid[pos.x][pos.y];
    color_t color = tile->meeple_color;
    tile->has_meeple      = false;
    tile->meeple_position = NO_DIRECTION;
    tile->meeple_color    = NO_COLOR;
    for (int i = 0; i < players->num_players; i++) {
        if (players->players[i].meeple_color == color) {
            players->players[i].meeples++;
            return;
        }
    }
    fprintf(stderr, "⚠️ Avertissement : aucune couleur correspondante trouvée pour rendre un meeple.\n");
}

/* =========== FRONTIÈRES DU PLATEAU =========== */
frontiers_t* create_frontier(void) {
    frontiers_t* frontier = malloc(sizeof *frontier);
    if (!frontier) {
        fprintf(stderr, "Erreur : allocation de frontiers_t échouée.\n");
        exit(EXIT_FAILURE);
    }
    frontier->size = 0;
    frontier->capacity = NB_TILES;
    frontier->positions = malloc(frontier->capacity * sizeof(position_t));
    if (!frontier->positions) {
        fprintf(stderr, "Erreur : allocation du tableau de positions échouée.\n");
        free(frontier);
        exit(EXIT_FAILURE);
    }
    return frontier;
}

void free_frontier(frontiers_t** frontier_ptr) {
    if (!frontier_ptr || !*frontier_ptr) return;
    free((*frontier_ptr)->positions);
    free(*frontier_ptr);
    *frontier_ptr = NULL;
}

bool is_in_frontier(frontiers_t* frontier, position_t pos) {
    for (int i = 0; i < frontier->size; i++) {
        if (frontier->positions[i].x == pos.x && frontier->positions[i].y == pos.y)
            return true;
    }
    return false;
}

void add_to_frontier(frontiers_t* frontier, position_t pos) {
    if (!is_in_frontier(frontier, pos)) {
        if (frontier->size < frontier->capacity) {
            frontier->positions[frontier->size++] = pos;
        } else {
            fprintf(stderr, "Erreur : frontière pleine, impossible d'ajouter (%d, %d)\n", pos.x, pos.y);
        }
    }
}

void remove_from_frontier(frontiers_t* frontier, position_t pos) {
    for (int i = 0; i < frontier->size; i++) {
        if (frontier->positions[i].x == pos.x && frontier->positions[i].y == pos.y) {
            frontier->positions[i] = frontier->positions[--frontier->size];
            return;
        }
    }
}

void update_frontier_after_placement(board_t* board, frontiers_t* frontier, position_t pos) {
    position_t directions[4] = { {-1,0}, {0,1}, {1,0}, {0,-1} };
    for (int i = 0; i < 4; i++) {
        position_t neighbor = { pos.x + directions[i].x, pos.y + directions[i].y };
        if (neighbor.x >= 0 && neighbor.x < board->height &&
            neighbor.y >= 0 && neighbor.y < board->width &&
            is_cell_empty(board, neighbor)) {
            add_to_frontier(frontier, neighbor);
        }
    }
}

/* =========== POSITIONS VALIDES =========== */
valid_positions_t* create_valid_positions(void) {
    valid_positions_t* vp = malloc(sizeof *vp);
    if (!vp) {
        fprintf(stderr, "Erreur : allocation de valid_positions_t échouée.\n");
        exit(EXIT_FAILURE);
    }

    vp->size = 0;
    vp->capacity = NB_TILES;
    vp->positions = malloc(vp->capacity * sizeof(position_t));
    if (!vp->positions) {
        fprintf(stderr, "Erreur : allocation du tableau de positions valides échouée.\n");
        free(vp);
        exit(EXIT_FAILURE);
    }

    return vp;
}

void destroy_valid_positions(valid_positions_t** vp_ptr) {
    if (!vp_ptr || !*vp_ptr) return;
    free((*vp_ptr)->positions);
    free(*vp_ptr);
    *vp_ptr = NULL;
}

void reset_valid_positions(valid_positions_t* vp) {
    if (vp) vp->size = 0;
}

void add_valid_position(valid_positions_t* vp, position_t pos) {
    if (vp->size == vp->capacity) {
        vp->capacity *= 2;
        position_t* tmp = realloc(vp->positions, vp->capacity * sizeof(position_t));
        if (!tmp) {
            fprintf(stderr, "Erreur : realloc échouée dans add_valid_position.\n");
            exit(EXIT_FAILURE);
        }
        vp->positions = tmp;
    }
    vp->positions[vp->size++] = pos;
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


void get_placement_tile(board_t* board,frontiers_t* frontiere,tile_t tile,degree_t rot,valid_positions_t* v)
{
    // tourne une copie selon rot
    tile_t tmp = tile;
    for (int r = 0; r < (int)rot; r++) {
    rotate_tile(&tmp, NINETY);
    }
    reset_valid_positions(v);
    // teste uniquement la rotation courante
    for (int i = 0; i < frontiere->size; i++) {
        position_t p = frontiere->positions[i];
        if (check_matching_zones(board, tmp, p)) {
            add_valid_position(v, p);
        }
    }
}

game_t init_game(int H, int W, const char* fn) {
    game_t G;
    G.board           = create_board(H, W);
    G.frontier        = create_frontier();
    G.valid_positions = create_valid_positions();
    G.tile_pack       = initialize_tile_pack(fn);
    tile_t st         = pick_tile(&G.tile_pack);
    position_t c      = { H/2, W/2 };
    place_tile(G.board, G.frontier, st, c);
    shuffle_tile_pack(&G.tile_pack);
    G.players         = create_players();
    return G;
}

// Deux types sont considérés équivalents pour l'occupation d'une structure
bool feature_matches(zone_type_t a, zone_type_t b) {
    return a == b || (a == CITY && b == SHIELD) || (a == SHIELD && b == CITY);
}

bool dfs_has_meeple(board_t* board, int x, int y, direction_t dir, zone_type_t target, bool* visited) {
    int idx = (x * board->width + y) * 5 + dir;
    if (visited[idx]) return false;
    visited[idx] = true;

    tile_t* tile = &board->grid[x][y];

    // Meeple directement sur la zone actuelle
    if (tile->has_meeple && tile->meeple_position == dir)
        return true;

    // Cas indirect : le center est du même type ET contient un meeple
    if (dir != CENTER && tile->has_meeple &&
        tile->meeple_position == CENTER &&
        feature_matches(tile->zones[dir], tile->zones[CENTER]) &&
        feature_matches(tile->zones[CENTER], target)) {
        return true;
    }

    // Cas inverse : depuis le CENTER, un bord connecté contient un meeple
    if (dir == CENTER) {
        for (direction_t d = 0; d < CENTER; d++) {
            if (tile->has_meeple && tile->meeple_position == d &&
                feature_matches(tile->zones[CENTER], tile->zones[d]) &&
                feature_matches(tile->zones[d], target)) {
                return true;
            }
        }
    }

    // Nouvelle exploration interne via center : si center relie à d'autres directions
    if (dir != CENTER && feature_matches(tile->zones[dir], tile->zones[CENTER]) &&
        feature_matches(tile->zones[CENTER], target)) {
        for (direction_t d = 0; d < CENTER; d++) {
            if (d != dir && feature_matches(tile->zones[d], target)) {
                if (dfs_has_meeple(board, x, y, d, target, visited))
                    return true;
            }
        }
    }

    // Exploration standard depuis le center vers bords
    if (dir == CENTER || feature_matches(tile->zones[CENTER], target)) {
        for (direction_t d = 0; d < CENTER; d++) {
            if (d != dir && feature_matches(tile->zones[d], target)) {
                if (dfs_has_meeple(board, x, y, d, target, visited))
                    return true;
            }
        }
    }

    // Exploration vers tuiles voisines
    if (dir < CENTER) {
        const int DX[4] = {-1, 0, 1, 0};
        const int DY[4] = { 0, 1, 0, -1};
        const direction_t OPP[4] = {SOUTH, WEST, NORTH, EAST};

        int nx = x + DX[dir], ny = y + DY[dir];
        if (nx >= 0 && nx < board->height && ny >= 0 && ny < board->width) {
            tile_t* neighbor = &board->grid[nx][ny];
            if (feature_matches(neighbor->zones[OPP[dir]], target)) {
                if (dfs_has_meeple(board, nx, ny, OPP[dir], target, visited))
                    return true;
            }
        }
    }

    return false;
}


// Détermine les zones où un meeple peut être placé sur une tuile à la position donnée
meeple_options_t get_placement_meeple(board_t* board, position_t pos) {
    meeple_options_t options = { .available = { false, false, false, false, false } };
    tile_t* tile = &board->grid[pos.x][pos.y];

    if (tile->has_meeple)
        return options;

    int total_size = board->height * board->width * 5;
    bool* visited = calloc(total_size, sizeof(bool));

    for (int z = 0; z < 5; z++) {
        zone_type_t zone_type = tile->zones[z];
        if (zone_type == NONE) continue;

        if (zone_type == ABBEY) {
            options.available[z] = true;
        } else if (zone_type == ROAD || zone_type == CITY || zone_type == SHIELD) {
            memset(visited, 0, total_size);
            if (!dfs_has_meeple(board, pos.x, pos.y, (direction_t)z, zone_type, visited)) {
                options.available[z] = true;
            }
        }
    }

    free(visited);
    return options;
}


void place_tile(board_t* bd, frontiers_t* f, tile_t t, position_t p) {
    bd->grid[p.x][p.y] = t;
    if(f) {
        update_frontier_after_placement(bd, f, p);
        if(is_in_frontier(f, p)) remove_from_frontier(f, p);
    }
}


static bool has_tile(board_t* bd, int x, int y) {
    for (int d = 0; d < 5; d++) {
        if (bd->grid[x][y].zones[d] != NONE) return true;
    }
    return false;
}

static int get_major_player(int* mcount, int nb_players) {
    int max = 0;
    for (int i = 0; i < nb_players; i++) if (mcount[i] > max) max = mcount[i];
    return max;
}

static void remove_visited_meeples(board_t* bd, game_players_t* gp, bool* zvis, int total_z, int W) {
    for (int i = 0; i < total_z; i++) {
        if (!zvis[i]) continue;
        int tid = i / 5;
        int dir = i % 5;
        int x = tid / W, y = tid % W;
        tile_t* t = &bd->grid[x][y];
        if (t->has_meeple && t->meeple_position == dir) {
            remove_meeple(bd, gp, (position_t){x, y});
        }
    }
}

static void score_structure_by_zones(game_t* game, zone_type_t target, bool complete_mode) {
    board_t* bd = game->board;
    game_players_t* gp = &game->players;
    int H = bd->height, W = bd->width, total_z = H * W * 5;

    static const int DX4[4] = {-1, 0, 1, 0};
    static const int DY4[4] = {0, 1, 0, -1};
    static const int OPP4[4] = {SOUTH, WEST, NORTH, EAST};

    bool* zvis = calloc(total_z, sizeof(bool));

    for (int idx = 0; idx < total_z; idx++) {
        if (zvis[idx]) continue;
        int tile_id = idx / 5;
        int z = idx % 5;
        int x = tile_id / W, y = tile_id % W;

        tile_t* t = &bd->grid[x][y];
        if (t->zones[z] != target) continue;

        int* q = malloc(total_z * sizeof(int));
        bool* tvis = calloc(H * W, sizeof(bool));
        int* mcount = calloc(gp->num_players, sizeof(int));
        int qh = 0, qt = 0, tcount = 0, scount = 0;
        bool closed = true;

        zvis[idx] = true;
        q[qt++] = idx;

        while (qh < qt) {
            int i = q[qh++];
            int tid = i / 5, dir = i % 5;
            int cx = tid / W, cy = tid % W;
            tile_t* ct = &bd->grid[cx][cy];

            if (!tvis[tid]) {
                tvis[tid] = true;
                tcount++;
                if (target == CITY) {
                    for (int d = 0; d < 5; d++) if (ct->zones[d] == SHIELD) scount++;
                }
            }

            if (ct->has_meeple && ct->meeple_position == dir) {
                for (int p = 0; p < gp->num_players; p++) {
                    if (gp->players[p].meeple_color == ct->meeple_color)
                        mcount[p]++;
                }
            }

            // Intra-tile connexions
            for (int d = 0; d < 5; d++) {
                int nid = tid * 5 + d;
                if (zvis[nid]) continue;
                if (ct->zones[d] != target) continue;

                // Connexions logiques internes via le center
                if (d == CENTER || dir == CENTER || ct->zones[CENTER] == target) {
                    zvis[nid] = true;
                    q[qt++] = nid;
                }
            }

            // Inter-tile connexions
            if (dir < 4) {
                int nx = cx + DX4[dir], ny = cy + DY4[dir];
                if (nx < 0 || nx >= H || ny < 0 || ny >= W ||
                    bd->grid[nx][ny].zones[OPP4[dir]] != target) {
                    closed = false;
                } else {
                    int nidx = (nx * W + ny) * 5 + OPP4[dir];
                    if (!zvis[nidx]) {
                        zvis[nidx] = true;
                        q[qt++] = nidx;
                    }
                }
            }
        }

        int best = get_major_player(mcount, gp->num_players);
        if (best > 0 && (!complete_mode || closed)) {
            int pts = (target == ROAD) ? tcount : (complete_mode ? tcount * 2 + scount * 2 : tcount + scount);
            const char* label = (target == ROAD ? "route" : "ville");

            for (int p = 0; p < gp->num_players; p++) {
                if (mcount[p] == best) {
                    gp->players[p].score += pts;
                    printf("Joueur %s marque %d pts (%s%s)\n",
                           gp->players[p].name, pts, label, complete_mode ? "" : " incomplète");
                }
            }

            remove_visited_meeples(bd, gp, zvis, total_z, W);
        }

        free(q);
        free(tvis);
        free(mcount);
    }

    free(zvis);
}


static void score_abbayes(game_t* game, bool complete_mode) {
    board_t* bd = game->board;
    game_players_t* gp = &game->players;
    int H = bd->height, W = bd->width;

    for (int x = 0; x < H; x++) {
        for (int y = 0; y < W; y++) {
            tile_t* t = &bd->grid[x][y];
            int idx = (x * W + y) * 5 + CENTER;
            if (scored[idx] || t->zones[CENTER] != ABBEY) continue;

            int count = 1;
            bool complete = true;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int xx = x + dx, yy = y + dy;
                    if (xx < 0 || xx >= H || yy < 0 || yy >= W || !has_tile(bd, xx, yy)) {
                        complete = false;
                        if (complete_mode) break;
                    }
                    if (!complete_mode && has_tile(bd, xx, yy)) count++;
                }
            }

            if (complete_mode && !complete) continue;

            if (t->has_meeple && t->meeple_position == CENTER) {
                for (int p = 0; p < gp->num_players; p++) {
                    if (gp->players[p].meeple_color == t->meeple_color) {
                        int pts = complete_mode ? 9 : count;
                        gp->players[p].score += pts;
                        printf("Joueur %s marque %d pts (%s)\n", gp->players[p].name, pts,
                               complete_mode ? "abbaye" : "abbaye incomplète");
                        break;
                    }
                }
                remove_meeple(bd, gp, (position_t){x, y});
                scored[idx] = true;
            }
        }
    }
}

// === STRUCTURES PRINCIPALES ===

void score_completed_structures(game_t* game, position_t last_pos) {
    board_t* bd = game->board;
    int H = bd->height, W = bd->width;
    int total_z = H * W * 5;
    if (!scored) {
        scored = calloc(total_z, sizeof(bool));
        if (!scored) { perror("calloc"); exit(EXIT_FAILURE); }
    }

    score_abbayes(game, true);
    score_structure_by_zones(game, ROAD, true);
    score_structure_by_zones(game, CITY, true);
}

void score_incomplete_structures(game_t* game) {
    board_t* bd = game->board;
    int H = bd->height, W = bd->width;
    int total_z = H * W * 5;
    if (!scored) {
        scored = calloc(total_z, sizeof(bool));
        if (!scored) { perror("calloc"); exit(EXIT_FAILURE); }
    }

    score_abbayes(game, false);
    score_structure_by_zones(game, ROAD, false);
    score_structure_by_zones(game, CITY, false);
}
