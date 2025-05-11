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

//Fisher-Yates shuffle
void shuffle_tile_pack(tiles_pack_t* pack) {
    for (int i = pack->remaining_tiles - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        tile_t tmp = pack->tiles[i];
        pack->tiles[i] = pack->tiles[j];
        pack->tiles[j] = tmp;
    }
}
game_players_t create_players() {
    game_players_t gp = {0};
    color_t colors[5] = {RED, BLUE, GREEN, YELLOW, WHITE};
    int human_count = 0, ai_count = 0;

    // === Saisie nombre de joueurs humains ===
    do {
        printf("Combien de joueurs humains ? (1 à 5) : ");
        if (scanf("%d", &human_count) != 1 || human_count < 1 || human_count > 5) {
            printf("❌ Saisie invalide. Veuillez entrer un nombre entre 1 et 5.\n");
            while (getchar() != '\n'); // vider buffer
            human_count = 0;
        } else {
            while (getchar() != '\n'); // vider buffer
        }
    } while (human_count == 0);

    // === Gestion spéciale si un seul joueur ===
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
    }
    // === Saisie IA facultative si joueurs humains < 5 ===
    else if (human_count < 5) {
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

    // === Création des joueurs humains ===
    for (int i = 0; i < human_count; i++) {
        printf("Entrez le nom du joueur %d : ", i + 1);
        fgets(gp.players[i].name, sizeof(gp.players[i].name), stdin);
        gp.players[i].name[strcspn(gp.players[i].name, "\n")] = '\0';

        gp.players[i].meeple_color = colors[i];
        gp.players[i].meeples = NB_MEEPLES;
        gp.players[i].is_ai = false;
        gp.players[i].score = 0;
    }

    // === Création des IA ===
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

/* === FRONTIERES ===  */
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

/* ==================  */

/* ====== MEEPLES =====*/
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
/* ==================  */

/* ====== POSITIONS VALIDES =====*/
// Allocation d'une structure valid_positions
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

// Libération mémoire
void destroy_valid_positions(valid_positions_t** vp_ptr) {
    if (!vp_ptr || !*vp_ptr) return;
    free((*vp_ptr)->positions);
    free(*vp_ptr);
    *vp_ptr = NULL;
}

// Réinitialisation (sans libérer)
void reset_valid_positions(valid_positions_t* vp) {
    if (vp) vp->size = 0;
}

// Ajout d'une position, avec redimensionnement si nécessaire
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
/* ==================  */

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
for (int i = 0; i < frontier->size; i++) {
position_t p = frontier->positions[i];
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



static bool has_tile(board_t* bd, int x, int y) {
    for (int d = 0; d < 5; d++) {
        if (bd->grid[x][y].zones[d] != NONE)
            return true;
    }
    return false;
}

void score_completed_structures(game_t* game, position_t last_pos) {
    board_t* bd = game->board;
    game_players_t* gp = &game->players;
    int H = bd->height, W = bd->width, total_z = H*W*5;

    // allocation de scored une seule fois pour toute la partie
    if(!scored) {
        scored = calloc(total_z, sizeof(bool));
        if(!scored) { perror("calloc"); exit(EXIT_FAILURE); }
    }

    // ABBEYES
    for(int dx=-1; dx<=1; dx++) {
        for(int dy=-1; dy<=1; dy++) {

            int ax = last_pos.x+dx, ay = last_pos.y+dy;

            if(ax<0 || ax>=H || ay<0 || ay>=W) continue;

            int idx = (ax*W + ay)*5 + CENTER;
            // on passe si déjà scorée ou si ce n'est pas une abbaye
            if(scored[idx] || bd->grid[ax][ay].zones[CENTER] != ABBEY) continue;

            bool complete = true;
            // vérifier que toutes les 8 cases autour sont présentes
            for(int i=-1; i<=1 && complete; i++){
                for(int j=-1; j<=1; j++){
                    int xx = ax + i, yy = ay + j;
                    if(xx<0 || xx>=H || yy<0 || yy>=W || !has_tile(bd, xx, yy)){
                        complete = false; break;
                    }
                }
            }
            // si l'abbaye n'est pas complète, on passe
            if(!complete) continue;

            // marquer l'abbaye comme scorée
            scored[idx] = true;

            tile_t* abb = &bd->grid[ax][ay];
            // vérifier la présence d’un meeple
            if(abb->has_meeple && abb->meeple_position == CENTER) {
                // trouver le joueur correspondant à la couleur du meeple
                for(int pi=0; pi<gp->num_players; pi++){
                    if(gp->players[pi].meeple_color == abb->meeple_color){
                        gp->players[pi].score += 9; // attribuer les 9 points
                        printf("Joueur %s marque 9 pts (abbaye[%d,%d])\n",
                            gp->players[pi].name, ax, ay);
                        break;
                    }
                }
                // retirer le meeple de l’abbaye
                remove_meeple(bd, gp, (position_t){ax, ay});
            }
        }
    }

    // ROUTES et VILLES
    static const int DX4[4]={-1,0,1,0}, DY4[4]={0,1,0,-1}, OPP4[4]={SOUTH,WEST,NORTH,EAST};
    // On explore les 4 côtés de la tuile posée
    for(int side=0; side<4; side++){
        zone_type_t type = bd->grid[last_pos.x][last_pos.y].zones[side];
        // vérifier si la case est une route ou une ville sinon on saute
        if(type != ROAD && type != CITY) continue;
        // on calcule l'index de la zone dans scored[]
        int start = (last_pos.x*W + last_pos.y)*5 + side;
        // si la zone a déjà été scorée, on passe à la suivante
        if(scored[start]) continue;

        // initialisation des structures de parcours
        bool *zvis = calloc(total_z, sizeof(bool));   // zones visitées
        bool *tvis = calloc(H*W, sizeof(bool));       // tuiles visitées
        int  *q    = malloc(total_z * sizeof(int));   // file de zones à visiter
        int qh = 0, qt = 0;                           // tête et queue de la file
        int tcount = 0, scount = 0;                   // nb de tuiles / boucliers
        int *mcount = calloc(gp->num_players, sizeof(int)); // meeples par joueur
        bool closed = true;                           // est-ce que la structure est fermée ?

        // on démarre le parcours depuis la zone start
        zvis[start] = true;
        q[qt++] = start;

        // parcours en largeur (BFS)
        while(qh < qt){
            int idx = q[qh++]; // défile la zone courante
            direction_t cz = (direction_t)(idx % 5); // direction courante
            int tid = idx / 5; // id de la tuile courante
            int x = tid / W, y = tid % W;  // coordonnées de la tuile courante
            tile_t* t = &bd->grid[x][y]; // pointeur vers la tuile courante

            // marquer la tuile comme visitée
            if(!tvis[tid]){
                tvis[tid] = true;
                tcount++; // incrémenter le nombre de tuiles de la structure
                if(type == CITY) {
                    // compter les boucliers si c’est une ville
                    for(int d2=0; d2<5; d2++)
                        if(t->zones[d2] == SHIELD) scount++;
                }
            }

            // si un meeple est placé sur cette zone
            if(t->has_meeple && t->meeple_position == cz) {
                for(int pi = 0; pi < gp->num_players; pi++)
                    if(gp->players[pi].meeple_color == t->meeple_color)
                        mcount[pi]++;
            }

            // intra-tile : chercher les autres zones connectées sur la même tuile
            for(direction_t dz = 0; dz < 5; dz++){
                if(dz == cz || zvis[tid*5 + dz]) continue;
                if(t->zones[dz] == type &&
                   (cz == CENTER || dz == CENTER || t->zones[CENTER] == type)){
                    zvis[tid*5 + dz] = true;
                    q[qt++] = tid*5 + dz;
                }
            }

            // inter-tile : vérifier les connexions avec les tuiles voisines
            if(cz < 4){
                int nx = x + DX4[cz], ny = y + DY4[cz];
                // si la tuile voisine est manquante ou mal connectée → structure ouverte
                if(nx < 0 || nx >= H || ny < 0 || ny >= W ||
                   bd->grid[nx][ny].zones[OPP4[cz]] != type){
                    closed = false;
                } else {
                    // sinon, on ajoute la zone correspondante de la tuile voisine
                    int nidx = (nx*W + ny)*5 + OPP4[cz];
                    if(!zvis[nidx]){
                        zvis[nidx] = true;
                        q[qt++] = nidx;
                    }
                }
            }
        }

        // si la structure est fermée, on peut attribuer les points
        if(closed){
            // marquer toutes les zones visitées comme scorées
            for(int i = 0; i < total_z; i++) if(zvis[i]) scored[i] = true;

            // déterminer le joueur majoritaire
            int best = 0;
            for(int pi = 0; pi < gp->num_players; pi++)
                if(mcount[pi] > best) best = mcount[pi];

            if(best > 0){
                // calcul des points selon le type
                int pts = type == ROAD ? tcount : (tcount*2 + scount*2);
                const char* kind = type == ROAD ? "route" : "ville";

                // donner les points à tous les joueurs majoritaires (égalité possible)
                for(int pi = 0; pi < gp->num_players; pi++){
                    if(mcount[pi] == best){
                        gp->players[pi].score += pts;
                        printf("Joueur %s marque %d pts (%s)\n",
                               gp->players[pi].name, pts, kind);
                    }
                }

                // retirer les meeples de toutes les tuiles concernées
                for(int tid = 0; tid < H*W; tid++){
                    if(tvis[tid]){
                        position_t pp = { tid/W, tid%W };
                        remove_meeple(bd, gp, pp);
                    }
                }
            }
        }

        // libération mémoire des structures temporaires
        free(zvis);
        free(tvis);
        free(q);
        free(mcount);
    }

}

void free_game(game_t* game) {
    free_frontier(&game->frontier);
    reset_valid_positions(game->valid_positions);
    destroy_board(game->board);
    free(game->players.players);
    reset_scored();
}
void score_incomplete_structures(game_t* game) {
    board_t* bd = game->board;
    game_players_t* gp = &game->players;
    int H = bd->height, W = bd->width, N = H * W;

    // --- Abbaye incomplète (inchangé) ---
    for (int x = 0; x < H; x++) {
        for (int y = 0; y < W; y++) {
            int idx_center = (x * W + y) * 5 + CENTER;
            if (!scored[idx_center] && bd->grid[x][y].zones[CENTER] == ABBEY) {
                // compter les tuiles adjacentes (3×3 autour)
                int adj_tiles = 0;
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        int xx = x + dx, yy = y + dy;
                        if (xx >= 0 && xx < H && yy >= 0 && yy < W && has_tile(bd, xx, yy))
                            adj_tiles++;
                    }
                }
                int pts = 1 + adj_tiles;
                printf("[DEBUG] abbaye incomplète en (%d,%d) : adj_tiles=%d → pts=%d\n",
                       x, y, adj_tiles, pts);

                // attribuer si meeple
                tile_t* abb = &bd->grid[x][y];
                if (abb->has_meeple && abb->meeple_position == CENTER) {
                    for (int p = 0; p < gp->num_players; p++) {
                        if (gp->players[p].meeple_color == abb->meeple_color) {
                            gp->players[p].score += pts;
                            printf("Joueur %s marque %d pts (abbaye incomplète)\n",
                                   gp->players[p].name, pts);
                            break;
                        }
                    }
                    remove_meeple(bd, gp, (position_t){x, y});
                }
                scored[idx_center] = true;
            }
        }
    }

    // --- Routes et villes incomplètes au niveau TUile ---
    bool *vis_city = calloc(N, sizeof(bool));
    bool *vis_road = calloc(N, sizeof(bool));

    const int DX[4]  = {-1, 0, 1, 0};
    const int DY[4]  = { 0, 1, 0,-1};
    const int OPP[4] = { SOUTH, WEST, NORTH, EAST };

    for (int kind = 0; kind < 2; kind++) {
        bool *vis       = (kind == 0 ? vis_city : vis_road);
        zone_type_t feat = (kind == 0 ? CITY : ROAD);
        const char* name = (kind == 0 ? "ville" : "route");

        for (int tid = 0; tid < N; tid++) {
            int x = tid / W, y = tid % W;
            tile_t* t = &bd->grid[x][y];

            // vérifier si la tuile a la feature et pas déjà visitée
            bool has_feat = false;
            for (int d = 0; d < 4; d++) {
                if (t->zones[d] == feat) { has_feat = true; break; }
            }
            if (!has_feat || vis[tid]) continue;

            // BFS tile→tile
            int *queue = malloc(N * sizeof(int));
            int qh = 0, qt = 0;
            queue[qt++] = tid;
            vis[tid]    = true;

            int tile_count = 0, shield_count = 0;
            int *mcount   = calloc(gp->num_players, sizeof(int));

            while (qh < qt) {
                int cur = queue[qh++];
                int cx  = cur / W, cy = cur % W;
                tile_t *ct = &bd->grid[cx][cy];

                tile_count++;
                if (feat == CITY) {
                    for (int d2 = 0; d2 < 5; d2++)
                        if (ct->zones[d2] == SHIELD) shield_count++;
                }
                if (ct->has_meeple) {
                    for (int p = 0; p < gp->num_players; p++) {
                        if (gp->players[p].meeple_color == ct->meeple_color)
                            mcount[p]++;
                    }
                }

                for (int d = 0; d < 4; d++) {
                    int nx = cx + DX[d], ny = cy + DY[d];
                    if (nx < 0 || nx >= H || ny < 0 || ny >= W) continue;
                    tile_t *nt = &bd->grid[nx][ny];
                    if (nt->zones[OPP[d]] != feat) continue;
                    int ntid = nx * W + ny;
                    if (!vis[ntid]) {
                        vis[ntid] = true;
                        queue[qt++] = ntid;
                    }
                }
            }

            free(queue);

            // debug & calcul points
            int pts = (feat == ROAD ? tile_count : tile_count + shield_count);
            printf("[DEBUG] %s incomplète : tcount=%d, scount=%d → pts=%d\n",
                   name, tile_count, shield_count, pts);

            // déterminer majorité
            int best = 0;
            for (int p = 0; p < gp->num_players; p++)
                if (mcount[p] > best) best = mcount[p];

            if (best > 0) {
                for (int p = 0; p < gp->num_players; p++) {
                    if (mcount[p] == best) {
                        gp->players[p].score += pts;
                        printf("Joueur %s marque %d pts (%s incomplète)\n",
                               gp->players[p].name, pts, name);
                    }
                }
                // retirer tous les meeples de cette structure
                for (int i = 0; i < N; i++) {
                    if (vis[i]) {
                        int ux = i / W, uy = i % W;
                        if (bd->grid[ux][uy].has_meeple)
                            remove_meeple(bd, gp, (position_t){ux, uy});
                    }
                }
            }

            free(mcount);
        }
    }

    free(vis_city);
    free(vis_road);
}
