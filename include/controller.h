#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "engine.h"
#include "gui.h"
#include <stdbool.h>

// Dimensions de la vue paginée
#define VIEW_WIDTH  10
#define VIEW_HEIGHT 5

/** Renvoie le code ANSI de la couleur d’un meeple */
void game_loop(game_t *game);

#endif // CONTROLLER_H