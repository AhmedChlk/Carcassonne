#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "controller.h"
#include "ansi_colors.h"
#include "gui.h"

// === CONSTANTES COULEURS ANSI POUR LES JOUEURS ===
static const char *player_colors[] = {
    [RED]      = ANSI_RED,
    [BLUE]     = ANSI_BLUE,
    [GREEN]    = ANSI_GREEN,
    [YELLOW]   = ANSI_YELLOW,
    [WHITE]    = ANSI_WHITE,
    [NO_COLOR] = ANSI_RESET
};

// === UTILITAIRE COULEUR ===
const char* color_code(color_t c) {
    if (c >= RED && c <= WHITE) return player_colors[c];
    return ANSI_RESET;
}

// === AFFICHAGE PROGRESSIF UTILITAIRE ===
void slow_print(const char *str, useconds_t delay_us) {
    for (const char *p = str; *p; ++p) {
        putchar(*p);
        fflush(stdout);
        usleep(delay_us);
    }
    putchar('\n');
}

// === AFFICHAGE ACCUEIL ===
void display_welcome(void) {
    const char *art[] = {
        "  ____    _    ____   ____    _    ____ ____   ___  _   _ _   _ _____ ",
        " / ___|  / \\  |  _ \\ / ___|  / \\  / ___/ ___| / _ \\| \\ | | \\ | | ____|",
        "| |     / _ \\ | |_) | |     / _ \\ \\___ \\___ \\| | | |  \\| |  \\| |  _|  ",
        "| |___ / ___ \\|  _ <| |___ / ___ \\ ___) |__) | |_| | |\\  | |\\  | |___ ",
        " \\____/_/   \\_\\_| \\_\\\\____/_/   \\_\\____/____/ \\___/|_| \\_|_| \\_|_____|"
    };
    for (int i = 0; i < (int)(sizeof(art)/sizeof(*art)); i++) {
        slow_print(art[i], 5000);
    }

    slow_print("\n          Bienvenue dans Carcassonne !", 10000);
    slow_print("=============================================\n", 10000);
}

// === MENU PRINCIPAL ===
int display_menu(void) {
    int choice;
    do {
        slow_print("╔════════════════════════════════════╗", 5000);
        slow_print("║          Menu principal           ║", 5000);
        slow_print("╠════════════════════════════════════╣", 5000);
        slow_print("║ 1. Commencer une nouvelle partie  ║", 5000);
        slow_print("║ 2. Quitter                        ║", 5000);
        slow_print("╚════════════════════════════════════╝", 5000);
        printf("Entrez votre choix (1–2) : ");

        if (scanf("%d", &choice) != 1) choice = 0;
        while (getchar() != '\n');
    } while (choice < 1 || choice > 2);

    return choice;
}
// === AFFICHAGE DU TOUR ET DES SCORES ===
static void display_turn_header(const char *player_name,
    color_t col,
    bool tileInHand,
    degree_t handRot,
    int remaining_meeples)
{
const char *ansi = color_code(col);
printf("%s>>> Tour de %s%s", ansi, player_name, ANSI_RESET);
if (tileInHand) printf(" | Rot %d°", handRot * 90);
printf(" | Meeples : %d\n", remaining_meeples);
}

static void display_scores(game_t *game) {
    printf("Scores :");
    for (int i = 0; i < game->players.num_players; i++) {
        player_t *p = &game->players.players[i];
        printf(" %s=%d", p->name, p->score);
    }
    printf("\n\n");

}

// === BOUCLE PRINCIPALE DU JEU ===
void game_loop(game_t *game) {
    bool game_over = false;
    int current = 0;
    int centerX = game->board->height / 2;
    int centerY = game->board->width / 2;
    int viewX = centerY - VIEW_WIDTH / 2;
    int viewY = centerX - VIEW_HEIGHT / 2;

    if (viewX < 0) viewX = 0;
    if (viewY < 0) viewY = 0;
    if (viewX > game->board->width - VIEW_WIDTH)
        viewX = game->board->width - VIEW_WIDTH;
    if (viewY > game->board->height - VIEW_HEIGHT)
        viewY = game->board->height - VIEW_HEIGHT;

    bool tileInHand = false;
    bool awaitingMeeple = false;
    tile_t handOrigTile;
    degree_t handRot = ZERO;
    position_t cursor = { centerX, centerY };
    meeple_options_t currentOpts;

    while (!game_over) {
        system("clear");
        player_t *p = &game->players.players[current];
        display_turn_header(p->name, p->meeple_color, tileInHand, handRot, p->meeples);
        display_scores(game);

        if (tileInHand && !awaitingMeeple) {
            get_placement_tile(game->board, game->frontier, handOrigTile, handRot, game->valid_positions);
            gui_set_valid_positions(game->valid_positions);
        } else {
            gui_set_valid_positions(NULL);
        }

        gui_set_remaining_tiles(game->tile_pack.remaining_tiles);
        print_board(game->board, viewX, viewY, VIEW_WIDTH, VIEW_HEIGHT, cursor);
        if (tileInHand)
            gui_show_current_tile(handOrigTile, handRot);

        // === AFFICHAGE DES COMMANDES ===
        if (!tileInHand) {
            printf("\nZQSD=±1 | ↑↓←→=±10 | Entrée=pioche");
        } else if (!awaitingMeeple) {
            printf("\nZQSD=±1 | ↑↓←→=±10 | R=rotate | Entrée=place");
        } else {
            printf("\nM=choix meeple | N=skip meeple");
        }

        // === GESTION DES ENTRÉES UTILISATEUR ===
        char buf[4] = {0};
        int n = 0, ch;
        while ((ch = getchar()) != '\n' && n < 4) buf[n++] = ch;
        char cmd = '\0';
        if (n >= 2 && buf[0] == '\033' && buf[1] == '[') {
            switch (buf[2]) {
                case 'A': cmd = 'U'; break;
                case 'B': cmd = 'D'; break;
                case 'C': cmd = 'R'; break;
                case 'D': cmd = 'L'; break;
            }
        } else if (n == 1) {
            cmd = tolower(buf[0]);
        } else {
            cmd = '\n';
        }

        // === DÉPLACEMENT CURSEUR ===
        int dx = 0, dy = 0;
        switch (cmd) {
            case 'z': dx = -1; break;
            case 's': dx = +1; break;
            case 'q': dy = -1; break;
            case 'd': dy = +1; break;
            case 'U': dx = -VIEW_HEIGHT; break;
            case 'D': dx = +VIEW_HEIGHT; break;
            case 'L': dy = -VIEW_WIDTH; break;
            case 'R': dy = +VIEW_WIDTH; break;
        }

        if (dx || dy) {
            int nx = cursor.x + dx, ny = cursor.y + dy;
            nx = nx < 0 ? 0 : (nx >= game->board->height ? game->board->height - 1 : nx);
            ny = ny < 0 ? 0 : (ny >= game->board->width ? game->board->width - 1 : ny);
            cursor.x = nx;
            cursor.y = ny;

            if (cursor.x < viewY) viewY = cursor.x;
            if (cursor.x >= viewY + VIEW_HEIGHT) viewY = cursor.x - VIEW_HEIGHT + 1;
            if (cursor.y < viewX) viewX = cursor.y;
            if (cursor.y >= viewX + VIEW_WIDTH) viewX = cursor.y - VIEW_WIDTH + 1;
            continue;
        }

        // === ACTIONS JEU ===
        if (!tileInHand && cmd == '\n') {
            handOrigTile = pick_tile(&game->tile_pack);
            handRot = ZERO;
            tileInHand = true;
            awaitingMeeple = false;
            continue;
        }

        if (tileInHand && !awaitingMeeple) {
            if (cmd == 'r') {
                handRot = (degree_t)((handRot + 1) % 4);
                continue;
            }
            if (cmd == '\n') {
                bool ok = false;
                for (int i = 0; i < game->valid_positions->actual; i++) {
                    position_t vp = game->valid_positions->valid_positions[i];
                    if (vp.x == cursor.x && vp.y == cursor.y) {
                        ok = true;
                        break;
                    }
                }
                if (!ok) {
                    slow_print(ANSI_RED "Position invalide, choisissez une autre." ANSI_RESET, 3000);
                    slow_print("Appuyez sur Entrée ↵ .", 3000);
                    while(getchar() != '\n');
                    continue;
                }
                tile_t toPlace = handOrigTile;
                for (int r = 0; r < (int)handRot; r++)
                    rotate_tile(&toPlace, NINETY);
                place_tile(game->board, game->frontier, toPlace, cursor);
                game->tile_pack.remaining_tiles--;
                awaitingMeeple = true;
                currentOpts = get_placement_meeple(game->board, cursor);
                continue;
            }
        }

        // === PLACEMENT MEEPLE ===
        if (awaitingMeeple) {
            if (p->meeples == 0) {
                slow_print(ANSI_YELLOW "⚠️  Plus de meeples ! Appuyez sur Entrée ↵ ." ANSI_RESET, 3000);
                while (getchar() != '\n');
                awaitingMeeple = false;
                tileInHand = false;
                score_completed_structures(game, cursor);
                current = (current + 1) % game->players.num_players;
                continue;
            }
            if (cmd == 'm') {
                gui_display_meeple_options(currentOpts, p->meeple_color);
                char msg[128];
                snprintf(msg, sizeof(msg),
                         ANSI_CYAN "Choix Meeple (reste %d) : [N]Nord [S]Sud [E]Est [W]Ouest [C]Centre" ANSI_RESET,
                         p->meeples);
                slow_print(msg, 3000);
                fflush(stdout);
                char d;
                do { d = tolower(getchar()); } while (strchr("nsewc", d) == NULL);
                while (getchar() != '\n');

                direction_t dir = (d == 'n' ? NORTH :
                                   d == 's' ? SOUTH :
                                   d == 'e' ? EAST :
                                   d == 'w' ? WEST : CENTER);
                if (currentOpts.available[dir]) {
                    place_meeple(game->board, &game->players, dir, cursor, p->meeple_color);
                } else {
                    slow_print(ANSI_RED "Option invalide." ANSI_RESET, 3000);
                }
                awaitingMeeple = false;
                tileInHand = false;
                score_completed_structures(game, cursor);
                current = (current + 1) % game->players.num_players;
                continue;
            }
            if (cmd == 'n') {
                awaitingMeeple = false;
                tileInHand = false;
                score_completed_structures(game, cursor);
                current = (current + 1) % game->players.num_players;
                continue;
            }
        }

        if (game->tile_pack.remaining_tiles == 0 && !tileInHand && !awaitingMeeple)
            game_over = true;
    }

    // === FIN DU JEU ===
    system("clear");
    final_scoring(game);

    slow_print(ANSI_BOLD ANSI_YELLOW "\n╔═══════════════════════╗" ANSI_RESET, 2000);
    slow_print(ANSI_BOLD ANSI_YELLOW "║     FIN DE PARTIE     ║" ANSI_RESET, 2000);
    slow_print(ANSI_BOLD ANSI_YELLOW "╚═══════════════════════╝" ANSI_RESET, 2000);
    slow_print("Classement final :", 2000);

    int n = game->players.num_players;
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (game->players.players[j].score > game->players.players[i].score) {
                player_t tmp = game->players.players[i];
                game->players.players[i] = game->players.players[j];
                game->players.players[j] = tmp;
            }
        }
    }

    for (int r = 0; r < n; r++) {
        player_t *p = &game->players.players[r];
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%2d. %s%s%s : %d pts%s%s",
                 r + 1,
                 color_code(p->meeple_color), p->name, ANSI_RESET,
                 p->score,
                 (r == 0 ? ANSI_BOLD ANSI_GREEN "  << VAINQUEUR" ANSI_RESET : ""),
                 ANSI_RESET);
        slow_print(buffer, 3000);
    }
    printf("\n");
}
