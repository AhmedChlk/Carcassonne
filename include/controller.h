#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "engine.h"
#include "gui.h"
#include <stdbool.h>

// === CONSTANTES DE VUE ===
#define VIEW_WIDTH  10
#define VIEW_HEIGHT 5

// === CODES ANSI POUR LES COULEURS DES MEEPLES ===
#define ANSI_RED    "\x1b[31m"
#define ANSI_GREEN  "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE   "\x1b[34m"
#define ANSI_WHITE  "\x1b[37m"
#define ANSI_RESET  "\x1b[0m"

// === UTILITAIRES DE COULEUR ===
/** Renvoie le code ANSI de la couleur d’un meeple */
const char* color_code(color_t c);

// === INTERFACE UTILISATEUR ===
/** Affiche l’écran d’accueil */
void display_welcome(void);

/** Affiche le menu principal et retourne 1 ou 2 */
int display_menu(void);

// === BOUCLE PRINCIPALE ===
/** Boucle principale du jeu */
void game_loop(game_t *game);

#endif // CONTROLLER_H
