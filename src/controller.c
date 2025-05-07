#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "controller.h"
#include "gui.h"

// ANSI pour couleurs joueurs
static const char *player_colors[] = {
    [RED]      = ANSI_RED,
    [BLUE]     = ANSI_BLUE,
    [GREEN]    = ANSI_GREEN,
    [YELLOW]   = ANSI_YELLOW,
    [WHITE]    = ANSI_WHITE,
    [NO_COLOR] = ANSI_RESET
};

const char* color_code(color_t c) {
    if (c >= RED && c <= WHITE) return player_colors[c];
    return ANSI_RESET;
}

void display_welcome(void) {
    const char *art[] = {
        "  ____    _    ____   ____    _    ____ ____   ___  _   _ _   _ _____ ",
        " / ___|  / \\  |  _ \\ / ___|  / \\  / ___/ ___| / _ \\| \\ | | \\ | | ____|",
        "| |     / _ \\ | |_) | |     / _ \\ \\___ \\___ \\| | | |  \\| |  \\| |  _|  ",
        "| |___ / ___ \\|  _ <| |___ / ___ \\ ___) |__) | |_| | |\\  | |\\  | |___ ",
        " \\____/_/   \\_\\_| \\_\\\\____/_/   \\_\\____/____/ \\___/|_| \\_|_| \\_|_____|"
    };
    for (int i = 0; i < (int)(sizeof(art)/sizeof(*art)); i++)
        printf("%s\n", art[i]);
    printf("\n          Bienvenue dans Carcassonne !\n");
    printf("=============================================\n\n");
}

int display_menu(void) {
    int choice;
    do {
        printf("╔════════════════════════════════════╗\n");
        printf("║          Menu principal           ║\n");
        printf("╠════════════════════════════════════╣\n");
        printf("║ 1. Commencer une nouvelle partie  ║\n");
        printf("║ 2. Quitter                        ║\n");
        printf("╚════════════════════════════════════╝\n");
        printf("Entrez votre choix (1–2) : ");
        if (scanf("%d", &choice) != 1) choice = 0;
        while (getchar()!='\n');
    } while (choice < 1 || choice > 2);
    return choice;
}

static void display_turn_header(const char *player_name,
                                color_t col,
                                bool tileInHand,
                                degree_t handRot)
{
    const char *ansi = color_code(col);
    printf("%s>>> Tour de %s%s", ansi, player_name, ANSI_RESET);
    if (tileInHand) printf(" | Rot %d°", handRot * 90);
    printf("\n");
}

static void display_scores(game_t *game) {
    printf("Scores :");
    for (int i = 0; i < game->players.num_players; i++) {
        player_t *p = &game->players.players[i];
        printf(" %s=%d", p->name, p->score);
    }
    printf("\n\n");
}

void game_loop(game_t *game) {
    bool             game_over      = false;
    int              current        = 0;
    int              centerX        = game->board->height / 2;
    int              centerY        = game->board->width  / 2;
    int              viewX          = centerY - VIEW_WIDTH/2;
    int              viewY          = centerX - VIEW_HEIGHT/2;
    if (viewX < 0) viewX = 0;
    if (viewY < 0) viewY = 0;
    if (viewX > game->board->width - VIEW_WIDTH)
        viewX = game->board->width - VIEW_WIDTH;
    if (viewY > game->board->height - VIEW_HEIGHT)
        viewY = game->board->height - VIEW_HEIGHT;

    bool             tileInHand     = false;
    bool             awaitingMeeple = false;
    tile_t           handOrigTile;
    degree_t         handRot        = ZERO;
    position_t       cursor         = { centerX, centerY };
    meeple_options_t currentOpts;

    while (!game_over) {
        system("clear");
        player_t *p = &game->players.players[current];

        display_turn_header(p->name, p->meeple_color, tileInHand, handRot);
        display_scores(game);

        if (tileInHand && !awaitingMeeple) {
            get_placement_tile(
              game->board,
              game->frontier,
              handOrigTile,
              handRot,
              game->valid_positions);
            gui_set_valid_positions(game->valid_positions);
        } else {
            gui_set_valid_positions(NULL);
        }

        print_board(game->board,
                    viewX, viewY,
                    VIEW_WIDTH, VIEW_HEIGHT,
                    cursor);

        if (tileInHand) {
            gui_show_current_tile(handOrigTile, handRot);
        }

        if (!tileInHand) {
            printf("\nZQSD=±1 | ↑↓←→=±10 | Entrée=pioche\n");
        } else if (!awaitingMeeple) {
            printf("\nZQSD=±1 | ↑↓←→=±10 | R=rotate | Entrée=place\n");
        } else {
            printf("\nM=choix meeple | N=skip meeple\n");
        }

        char buf[4] = {0};
        int  n=0,ch;
        while ((ch=getchar())!='\n' && n<4) buf[n++]=ch;
        char cmd='\0';
        if (n>=2 && buf[0]=='\033'&&buf[1]=='[') {
            switch(buf[2]) {
                case 'A': cmd='U'; break;
                case 'B': cmd='D'; break;
                case 'C': cmd='R'; break;
                case 'D': cmd='L'; break;
            }
        } else if (n==1) {
            cmd = tolower(buf[0]);
        } else {
            cmd = '\n';
        }

        int dx=0, dy=0;
        switch(cmd) {
            case 'z': dx=-1; break;
            case 's': dx=+1; break;
            case 'q': dy=-1; break;
            case 'd': dy=+1; break;
            case 'U': dx=-VIEW_HEIGHT; break;
            case 'D': dx=+VIEW_HEIGHT; break;
            case 'L': dy=-VIEW_WIDTH;  break;
            case 'R': dy=+VIEW_WIDTH;  break;
        }
        if (dx||dy) {
            int nx=cursor.x+dx, ny=cursor.y+dy;
            if (nx<0) nx=0;
            if (nx>=game->board->height) nx=game->board->height-1;
            if (ny<0) ny=0;
            if (ny>=game->board->width)  ny=game->board->width-1;
            cursor.x=nx; cursor.y=ny;
            if (cursor.x<viewY) viewY=cursor.x;
            if (cursor.x>=viewY+VIEW_HEIGHT)
                viewY=cursor.x-VIEW_HEIGHT+1;
            if (cursor.y<viewX) viewX=cursor.y;
            if (cursor.y>=viewX+VIEW_WIDTH)
                viewX=cursor.y-VIEW_WIDTH+1;
            continue;
        }

        if (!tileInHand && cmd=='\n') {
            handOrigTile = pick_tile(&game->tile_pack);
            handRot      = ZERO;
            tileInHand   = true;
            awaitingMeeple = false;
            continue;
        }

        if (tileInHand &&

            !awaitingMeeple) {
            if (cmd=='r') {
                handRot = (degree_t)((handRot+1)%4);
                continue;
            }
            if (cmd=='\n') {
                bool ok=false;
                for (int i=0;i<game->valid_positions->actual;i++){
                    position_t vp=game->valid_positions->valid_positions[i];
                    if(vp.x==cursor.x&&vp.y==cursor.y){ ok=true; break; }
                }
                if (!ok) {
                    printf("Position invalide, choisissez autre.\n");
                    continue;
                }
                tile_t toPlace = handOrigTile;
                for (int r=0;r<handRot;r++)
                    rotate_tile(&toPlace, NINETY);
                place_tile(game->board,game->frontier,toPlace,cursor);
                game->tile_pack.remaining_tiles--;
                awaitingMeeple = true;
                currentOpts    = get_placement_meeple(game->board,cursor);
                continue;
            }
        }

        if (awaitingMeeple) {
            if (p->meeples==0) {
                printf("Plus de meeples ! Appuyez sur Entrée\n");
                while(getchar()!='\n');
                awaitingMeeple=false;
                tileInHand=false;
                score_completed_structures(game,cursor);
                current=(current+1)%game->players.num_players;
                continue;
            }
            if (cmd=='m') {
                gui_show_current_tile(handOrigTile,handRot);
                gui_display_meeple_options(currentOpts,p->meeple_color);
                printf("Choix Meeple (reste %d) : [N]Nord [S]Sud [E]Est [W]Ouest [C]Centre\n",
                       p->meeples);
                fflush(stdout);
                char d;
                do{ d=tolower(getchar()); }
                while(strchr("nsewc",d)==NULL);
                while(getchar()!='\n');
                direction_t dir = (d=='n'?NORTH:
                                   d=='s'?SOUTH:
                                   d=='e'?EAST:
                                   d=='w'?WEST:CENTER);
                if (currentOpts.available[dir]) {
                    place_meeple(game->board,&game->players,dir,cursor,p->meeple_color);
                } else {
                    printf("Option invalide.\n");
                }
                awaitingMeeple=false;
                tileInHand=false;
                score_completed_structures(game,cursor);
                current=(current+1)%game->players.num_players;
                continue;
            }
            if (cmd=='n') {
                awaitingMeeple=false;
                tileInHand=false;
                score_completed_structures(game,cursor);
                current=(current+1)%game->players.num_players;
                continue;
            }
        }

        if (game->tile_pack.remaining_tiles==0 && !tileInHand && !awaitingMeeple)
            game_over=true;
    }
}
