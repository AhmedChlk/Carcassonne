// src/engine.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "board.h"
#include "tile.h"
#include "engine.h"
#include "player.h"

// Initialise un plateau h×w
board_t* initialize_board(int h, int w) {
    board_t* bd = malloc(sizeof *bd);
    if (!bd) { fprintf(stderr,"Erreur alloc board\n"); exit(EXIT_FAILURE); }
    bd->height = h; bd->width = w;

    bd->grid = malloc(h * sizeof *bd->grid);
    if (!bd->grid) { fprintf(stderr,"Erreur alloc rows\n"); exit(EXIT_FAILURE); }
    for (int i = 0; i < h; i++) {
        bd->grid[i] = malloc(w * sizeof *bd->grid[i]);
        if (!bd->grid[i]) { fprintf(stderr,"Erreur alloc row %d\n",i); exit(EXIT_FAILURE); }
    }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            for (int k = 0; k < 5; k++)
                bd->grid[i][j].zones[k] = NONE;
            bd->grid[i][j].has_meeple     = false;
            bd->grid[i][j].meeple_position = NO_DIRECTION;
            bd->grid[i][j].meeple_color    = NO_COLOR;
        }
    }
    return bd;
}

// Libère un plateau
void free_board(board_t* bd) {
    for (int i = 0; i < bd->height; i++)
        free(bd->grid[i]);
    free(bd->grid);
    free(bd);
}

// Charge le pack depuis data/filename
tiles_pack_t initialize_tile_pack(const char* fn) {
    char path[256] = "data/";
    strcat(path, fn);
    FILE* f = fopen(path, "r");
    if (!f) { fprintf(stderr,"Erreur open %s\n",path); exit(EXIT_FAILURE); }
    tiles_pack_t tp;
    tp.remaining_tiles = NB_TILES;
    char buf[1024];
    int zi, ti = 0;
    while (fgets(buf, sizeof buf, f)) {
        zi = 0;
        char* tok = strtok(buf, ",\n");
        while (tok) {
            if (!strcmp(tok,"blason"))    tp.tiles[ti].zones[zi++] = SHIELD;
            else if (!strcmp(tok,"route")) tp.tiles[ti].zones[zi++] = ROAD;
            else if (!strcmp(tok,"pre"))   tp.tiles[ti].zones[zi++] = FIELD;
            else if (!strcmp(tok,"village")) tp.tiles[ti].zones[zi++] = VILLAGE;
            else if (!strcmp(tok,"ville")) tp.tiles[ti].zones[zi++] = CITY;
            else if (!strcmp(tok,"abbaye"))tp.tiles[ti].zones[zi++] = ABBEY;
            tok = strtok(NULL,",\n");
        }
        tp.tiles[ti].has_meeple     = false;
        tp.tiles[ti].meeple_position = NO_DIRECTION;
        tp.tiles[ti].meeple_color    = NO_COLOR;
        ti++;
    }
    fclose(f);
    return tp;
}

// Mélange aléatoire du pack
void shuffle_tile_pack(tiles_pack_t* tp) {
    for (int i = 0; i < NB_TILES; i++) {
        int a = rand()%NB_TILES, b;
        do { b = rand()%NB_TILES; } while (a==b);
        tile_t tmp = tp->tiles[a];
        tp->tiles[a] = tp->tiles[b];
        tp->tiles[b] = tmp;
    }
}

// Crée les joueurs
game_players_t create_players() {
    game_players_t gp;
    color_t cols[5] = {RED,BLUE,GREEN,YELLOW,WHITE};
    int np;
    printf("Nbr joueurs (1-5) : ");
    while (scanf("%d",&np)!=1 || np<1||np>5) {
        printf("Valeur invalide, retentez : ");
        while(getchar()!='\n');
    }
    while(getchar()!='\n');
    gp.num_players = np;
    gp.players = malloc(np*sizeof(player_t));
    for (int i = 0; i < np; i++) {
        printf("Nom joueur %d : ",i+1);
        fgets(gp.players[i].name,50,stdin);
        gp.players[i].name[strcspn(gp.players[i].name,"\n")] = 0;
        gp.players[i].meeple_color = cols[i];
        gp.players[i].meeples      = NB_MEEPLES;
        gp.players[i].is_ai        = false;
    }
    if (np<5) {
        char ans='n'; int na=0;
        if (np>1) {
            printf("Ajouter IA ? (y/n) : ");
            scanf(" %c",&ans);
            while(getchar()!='\n');
        }
        if (np==1||ans=='y'||ans=='Y') {
            printf("Nbr IA (max %d) : ",5-np);
            while(scanf("%d",&na)!=1||na<0||na+np>5||na+np<2) {
                printf("Valeur invalide, retentez : ");
                while(getchar()!='\n');
            }
            int base=np;
            gp.players = realloc(gp.players,(base+na)*sizeof(player_t));
            for (int i = 0; i < na; i++) {
                int idx = base+i;
                snprintf(gp.players[idx].name,16,"AI_%d",i+1);
                gp.players[idx].meeple_color = cols[idx];
                gp.players[idx].meeples      = NB_MEEPLES;
                gp.players[idx].is_ai        = true;
            }
            gp.num_players = base+na;
        }
    }
    return gp;
}

// Pioche la tuile du haut
tile_t pick_tile(tiles_pack_t* tp) {
    return tp->tiles[--tp->remaining_tiles];
}

// Tourne la tuile
void rotate_tile(tile_t* t, degree_t deg) {
    zone_type_t tmp;
    switch(deg){
        case NINETY:
            tmp=t->zones[NORTH];
            t->zones[NORTH]=t->zones[WEST];
            t->zones[WEST]=t->zones[SOUTH];
            t->zones[SOUTH]=t->zones[EAST];
            t->zones[EAST]=tmp;
            break;
        case ONE_HUNDRED_EIGHTY:
            tmp=t->zones[NORTH]; t->zones[NORTH]=t->zones[SOUTH]; t->zones[SOUTH]=tmp;
            tmp=t->zones[EAST];  t->zones[EAST]=t->zones[WEST];  t->zones[WEST]=tmp;
            break;
        case TWO_HUNDRED_SEVENTY:
            tmp=t->zones[NORTH];
            t->zones[NORTH]=t->zones[EAST];
            t->zones[EAST]=t->zones[SOUTH];
            t->zones[SOUTH]=t->zones[WEST];
            t->zones[WEST]=tmp;
            break;
        default: break;
    }
}

// Vérifie si case vide
bool is_cell_empty(board_t* bd, position_t p) {
    for(int i=0;i<5;i++) if(bd->grid[p.x][p.y].zones[i]!=NONE) return false;
    return true;
}

// Vérif frontier
bool is_in_frontier(frontiers_t* f, position_t p){
    for(int i=0;i<f->actual;i++)
        if(f->positions[i].x==p.x&&f->positions[i].y==p.y) return true;
    return false;
}

// Alloue frontier
frontiers_t* allocate_frontier(){
    frontiers_t* f=malloc(sizeof *f);
    f->actual=0; f->capacity=NB_TILES;
    f->positions=malloc(f->capacity*sizeof(position_t));
    return f;
}

// Libère frontier
void free_frontier(frontiers_t** f){
    free((*f)->positions);
    free(*f); *f=NULL;
}

// Ajoute frontier
void add_to_frontier(frontiers_t* f, position_t p){
    if(!is_in_frontier(f,p)) f->positions[f->actual++]=p;
}

// Retire frontier
void remove_from_frontier(frontiers_t* f, position_t p){
    int i=0;
    while(i<f->actual&&(f->positions[i].x!=p.x||f->positions[i].y!=p.y)) i++;
    if(i<f->actual) f->positions[i]=f->positions[--f->actual];
}

// Met à jour frontier
void update_frontier_after_placement(board_t* bd, frontiers_t* f, position_t p){
    position_t d[4]={{-1,0},{0,1},{1,0},{0,-1}};
    for(int k=0;k<4;k++){
        position_t q={p.x+d[k].x,p.y+d[k].y};
        if(q.x>=0&&q.x<bd->height&&q.y>=0&&q.y<bd->width&&is_cell_empty(bd,q))
            add_to_frontier(f,q);
    }
}

// Place tuile
void place_tile(board_t* bd, frontiers_t* f, tile_t t, position_t p){
    bd->grid[p.x][p.y]=t;
    update_frontier_after_placement(bd,f,p);
    if(is_in_frontier(f,p)) remove_from_frontier(f,p);
}

// Place meeple
void place_meeple(board_t* bd, game_players_t* gp, direction_t d, position_t p, color_t c){
    bd->grid[p.x][p.y].has_meeple=true;
    bd->grid[p.x][p.y].meeple_position=d;
    bd->grid[p.x][p.y].meeple_color=c;
    for(int i=0;i<gp->num_players;i++)
        if(gp->players[i].meeple_color==c) gp->players[i].meeples--;
}

// Retire meeple
void remove_meeple(board_t* bd, game_players_t* gp, position_t p){
    bd->grid[p.x][p.y].has_meeple=false;
    bd->grid[p.x][p.y].meeple_position=NO_DIRECTION;
    for (int i = 0; i < gp->num_players; i++)
        if (gp->players[i].meeple_color == bd->grid[p.x][p.y].meeple_color){
            gp->players[i].meeples++;
            bd->grid[p.x][p.y].meeple_color = NO_COLOR;
            break;
        }
}

// Alloue valid pos
valid_positions_t* allocate_valid_positions(){
    valid_positions_t* v=malloc(sizeof *v);
    v->actual=0; v->capacity=NB_TILES;
    v->valid_positions=malloc(v->capacity*sizeof(position_t));
    return v;
}

// Libère valid pos
void free_valid_positions(valid_positions_t** v){
    free((*v)->valid_positions);
    free(*v); *v=NULL;
}

// Reset valid pos
void reset_valid_positions(valid_positions_t* v){
    v->actual=0;
}

// Ajoute valid pos
void add_to_valid_positions(valid_positions_t* v, position_t p){
    if(v->actual==v->capacity){
        v->capacity*=2;
        v->valid_positions=realloc(v->valid_positions,v->capacity*sizeof(position_t));
    }
    v->valid_positions[v->actual++]=p;
}

// Vérif zones
bool check_matching_zones(board_t* bd, tile_t t, position_t p){
    position_t d[4]={{-1,0},{0,1},{1,0},{0,-1}};
    direction_t o[4]={SOUTH,WEST,NORTH,EAST};
    for(int k=0;k<4;k++){
        int nx=p.x+d[k].x, ny=p.y+d[k].y;
        if(nx>=0&&nx<bd->height&&ny>=0&&ny<bd->width){
            if(bd->grid[nx][ny].zones[o[k]]!=NONE){
                if(t.zones[k]!=bd->grid[nx][ny].zones[o[k]]){
                    if(!((t.zones[k]==SHIELD&&bd->grid[nx][ny].zones[o[k]]==CITY)||
                         (t.zones[k]==CITY&&bd->grid[nx][ny].zones[o[k]]==SHIELD)))
                        return false;
                }
            }
        }
    }
    return true;
}

// Remplit valid placements
void get_placement_tile(board_t* bd, frontiers_t* f, tile_t t, valid_positions_t* v){
    reset_valid_positions(v);
    for(int i=0;i<f->actual;i++){
        position_t p={f->positions[i].x,f->positions[i].y};
        tile_t tmp=t;
        for(int r=0;r<4;r++){
            if(check_matching_zones(bd,tmp,p)){
                add_to_valid_positions(v,p);
            }
            rotate_tile(&tmp,NINETY);
        }
    }
}

// Init partie
game_t init_game(int H,int W,const char* fn){
    game_t G;
    G.board=initialize_board(H,W);
    G.frontier=allocate_frontier();
    G.valid_positions=allocate_valid_positions();
    G.tile_pack=initialize_tile_pack(fn);
    tile_t st=pick_tile(&G.tile_pack);
    position_t c={H/2,W/2};
    place_tile(G.board,G.frontier,st,c);
    shuffle_tile_pack(&G.tile_pack);
    G.players=create_players();
    return G;
}

// Vérif correspondance
bool feature_matches(zone_type_t a, zone_type_t b){
    return a==b || (a==CITY&&b==SHIELD)||(a==SHIELD&&b==CITY);
}

// DFS de meeple
bool dfs_has_meeple(board_t* bd,int x,int y,direction_t z,zone_type_t tgt,bool* vs){
    int idx=(x*bd->width+y)*5+z;
    if(vs[idx]) return false;
    vs[idx]=true;
    tile_t* tl=&bd->grid[x][y];
    if(tl->has_meeple&&tl->meeple_position==z) return true;
    if(z==CENTER||feature_matches(tl->zones[CENTER],tgt)){
        for(int i=0;i<5;i++){
            direction_t d2=(direction_t)i;
            if(d2!=z&&feature_matches(tl->zones[i],tgt)){
                int idx2=(x*bd->width+y)*5+i;
                if(!vs[idx2]){
                    if(tl->has_meeple&&tl->meeple_position==d2) return true;
                    if(dfs_has_meeple(bd,x,y,d2,tgt,vs)) return true;
                }
            }
        }
    }
    if(z<CENTER){
        int nx=x,ny=y; direction_t op=NO_DIRECTION;
        switch(z){
            case NORTH: op=SOUTH; nx=x-1; break;
            case EAST:  op=WEST;  ny=y+1; break;
            case SOUTH: op=NORTH; nx=x+1; break;
            case WEST:  op=EAST;  ny=y-1; break;
            default: break;
        }
        if(op!=NO_DIRECTION&&nx>=0&&nx<bd->height&&ny>=0&&ny<bd->width
           &&feature_matches(bd->grid[nx][ny].zones[op],tgt)){
            int idx3=(nx*bd->width+ny)*5+op;
            if(!vs[idx3]){
                tile_t* nb=&bd->grid[nx][ny];
                if(nb->has_meeple&&nb->meeple_position==op) return true;
                if(dfs_has_meeple(bd,nx,ny,op,tgt,vs)) return true;
            }
        }
    }
    return false;
}
// Zones pour meeple
meeple_options_t get_placement_meeple(board_t* bd, position_t p){
    meeple_options_t opt={{false,false,false,false,false}};
    tile_t* tl=&bd->grid[p.x][p.y];
    if(tl->has_meeple) return opt;
    int ts=bd->height*bd->width*5;
    bool* vs=calloc(ts,sizeof(bool));
    for(int z=0;z<5;z++){
        zone_type_t tp=tl->zones[z];
        if(tp==NONE) continue;
        if(tp==ABBEY) opt.available[z]=true;
        else if(tp==ROAD||tp==CITY||tp==SHIELD){
            memset(vs,0,ts);
            if(!dfs_has_meeple(bd,p.x,p.y,(direction_t)z,tp,vs))
                opt.available[z]=true;
        }
    }
    free(vs);
    return opt;
}
