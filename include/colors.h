#ifndef COLORS_H
#define COLORS_H

#include "tile.h"

#define ANSI_RED    "\x1b[31m"
#define ANSI_GREEN  "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE   "\x1b[34m"
#define ANSI_WHITE  "\x1b[37m"
#define ANSI_RESET  "\x1b[0m"

#define BG_GREEN   "\033[42m"
#define REV_VIDEO  "\033[7m"

#define CYAN_BOLD    "\x1b[1;36m"
#define YELLOW_BOLD  "\x1b[1;33m"
#define WHITE_BOLD   "\x1b[1;37m"

const char* zone_color(zone_type_t z);              // int à la place de color_t ici
const char* color_code(color_t z);              // idem : on évite le type incomplet

#endif // COLORS_H
