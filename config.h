#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include "engine.h"
#include "players.h"

typedef enum {
    DISPLAY_CONSOLE = 1,
    DISPLAY_GUI     = 2
} DisplayMode;

typedef enum {
    MODE_PVP    = 1,
    MODE_AI     = 2,
    MODE_FILE   = 3,
    MODE_REPLAY = 4
} GameMode;

typedef struct {
    DisplayMode display_mode;
    GameMode    game_mode;
    int         rows;
    int         cols;
    int         ai_difficulty;
    char        file_path[256];
    char        replay_name[64];

    /* Runtime resources */
    FILE       *file_handle;
    int        *replay_moves;
    int         replay_move_count;
} GameConfig;

void config_print_usage(const char *progname);
int  config_load(GameConfig *cfg, int argc, char *argv[]);
int  config_setup_players(const GameConfig *cfg,
                          Player players[2],
                          AIContext *ai_ctx,
                          ReplayContext *replay_ctx,
                          DisplayFn *display_fn);
void config_cleanup(GameConfig *cfg);

#endif
