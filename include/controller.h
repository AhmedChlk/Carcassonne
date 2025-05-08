#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "engine.h"
#include "gui.h"
#include <stdbool.h>

// === CONSTANTES DE VUE ===
#define VIEW_WIDTH  10
#define VIEW_HEIGHT 5


typedef enum {
    CTRL_NONE,
    CTRL_PIOCHE,
    CTRL_PLACE,
    CTRL_MEEPLE
} controls_state_t;

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
