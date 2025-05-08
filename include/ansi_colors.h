#ifndef ANSI_COLORS_H
#define ANSI_COLORS_H

// === Couleurs de texte ===
#define ANSI_RESET    "\x1b[0m"
#define ANSI_BOLD     "\x1b[1m"

#define ANSI_BLACK    "\x1b[30m"
#define ANSI_RED      "\x1b[31m"
#define ANSI_GREEN    "\x1b[32m"
#define ANSI_YELLOW   "\x1b[33m"
#define ANSI_BLUE     "\x1b[34m"
#define ANSI_MAGENTA  "\x1b[35m"
#define ANSI_CYAN     "\x1b[36m"
#define ANSI_WHITE    "\x1b[37m"

// === Couleurs de fond ===
#define BG_BLACK      "\x1b[40m"
#define BG_RED        "\x1b[41m"
#define BG_GREEN      "\x1b[42m"
#define BG_YELLOW     "\x1b[43m"
#define BG_BLUE       "\x1b[44m"
#define BG_MAGENTA    "\x1b[45m"
#define BG_CYAN       "\x1b[46m"
#define BG_WHITE      "\x1b[47m"

// === Effets ===
#define REV_VIDEO     "\x1b[7m"
#define UNDERLINE     "\x1b[4m"

#endif // ANSI_COLORS_H
