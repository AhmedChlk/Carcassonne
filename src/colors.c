#include "colors.h"
#include "tile.h" 

static const char *player_colors[] = {
    [RED]      = ANSI_RED,
    [BLUE]     = ANSI_BLUE,
    [GREEN]    = ANSI_GREEN,
    [YELLOW]   = ANSI_YELLOW,
    [WHITE]    = ANSI_WHITE,
    [NO_COLOR] = ANSI_RESET
};

const char* color_code(color_t c) {
    if (c >= RED && c <= NO_COLOR) return player_colors[c];
    return ANSI_RESET;
}

const char* zone_color(zone_type_t z) {
    switch (z) {
        case CITY:    return "\033[0;31m";
        case ROAD:    return "\033[0;33m";
        case FIELD:   return "\033[0;32m";
        case SHIELD:  return "\033[0;35m";
        case VILLAGE: return "\033[0;36m";
        case ABBEY:   return "\033[0;37m";
        default:      return ANSI_RESET;
    }
}


