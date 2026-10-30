#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "config.h"
#include "database.h"
#include "gui.h"

static void flush_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void config_print_usage(const char *progname) {
    printf("Connect Four - Classic & Modern Strategy Game in C\n");
    printf("Usage: %s [OPTIONS]\n\n", progname);
    printf("Display & Game Options:\n");
    printf("  -g, --gui                Launch graphical SDL2 window mode\n");
    printf("  -c, --cli                Launch text/console mode\n");
    printf("  -p, --pvp                Human vs Human game mode\n");
    printf("  -a, --ai <level>         Human vs AI game mode (0=Easy, 1=Medium, 2=Hard)\n");
    printf("  -f, --file <path>        File input mode (read Player O moves from file)\n");
    printf("  -r, --replay [name]      Replay mode (optional save name to replay)\n");
    printf("  -d, --dim <RxC>          Board dimensions, e.g. 6x7 or 8x8 (Limits: 4-11)\n");
    printf("  -R, --rows <rows>        Number of board rows (4-11)\n");
    printf("  -C, --cols <cols>        Number of board columns (4-11)\n");
    printf("  -h, --help               Display this help guide and exit\n\n");
    printf("In-Game Controls:\n");
    printf("  Mouse Click / Numbers    Select column to drop piece\n");
    printf("  [u] / Ctrl+Z             Undo previous move\n");
    printf("  [q]                      Quit match\n");
}

static int prompt_display_mode(void) {
    int mode;
    printf("\nDisplay mode:\n");
    printf("  1. Console\n");
    printf("  2. GUI (SDL2)\n");
    printf("Select display (1-2): ");

    while (1) {
        int parsed = scanf("%d", &mode);
        flush_input();

        if (parsed == EOF) return -1;
        if (parsed == 1 && (mode == 1 || mode == 2)) return mode;
        printf("Please enter 1 or 2: ");
    }
}

static int prompt_game_mode(void) {
    int mode;
    printf("\nGame modes:\n");
    printf("  1. Human vs Human\n");
    printf("  2. Human vs AI\n");
    printf("  3. File input\n");
    printf("  4. Replay saved game\n");
    printf("Select mode (1-4): ");

    while (1) {
        int parsed = scanf("%d", &mode);
        flush_input();

        if (parsed == EOF) return -1;
        if (parsed == 1 && mode >= 1 && mode <= 4) return mode;
        printf("Please enter 1-4: ");
    }
}

static int prompt_dimensions(int *rows, int *cols) {
    while (1) {
        printf("Board dimensions (rows cols, 4-11): ");
        int parsed = scanf("%d %d", rows, cols);
        flush_input();

        if (parsed == EOF) return 0;
        if (parsed == 2 && *rows >= MIN_DIM && *rows <= MAX_DIM &&
                           *cols >= MIN_DIM && *cols <= MAX_DIM) {
            return 1;
        }
        fprintf(stderr, "error: dimensions must be between 4 and 11\n");
    }
}

static int prompt_ai_difficulty(void) {
    int diff;
    while (1) {
        printf("AI difficulty (0=Easy, 1=Medium, 2=Hard): ");
        int parsed = scanf("%d", &diff);
        flush_input();

        if (parsed == EOF) return -1;
        if (parsed == 1 && diff >= 0 && diff <= 2) return diff;
        fprintf(stderr, "error: invalid difficulty (0-2)\n");
    }
}

static int prompt_replay_selection(char *dest, size_t dest_size) {
    SavedGameInfo saves[50];
    int count = list_saved_games("database.dat", saves, 50);
    if (count == 0) {
        fprintf(stderr, "No saved games found to replay.\n");
        return 0;
    }

    printf("Select game [1-%d] or enter name: ", count);
    char input_buf[64];
    if (scanf("%63s", input_buf) != 1) {
        fprintf(stderr, "error reading selection\n");
        return 0;
    }
    flush_input();

    int selected_idx = atoi(input_buf);
    if (selected_idx >= 1 && selected_idx <= count) {
        snprintf(dest, dest_size, "%.*s", (int)(dest_size - 1), saves[selected_idx - 1].name);
    } else {
        snprintf(dest, dest_size, "%.*s", (int)(dest_size - 1), input_buf);
    }
    return 1;
}

int config_load(GameConfig *cfg, int argc, char *argv[]) {
    memset(cfg, 0, sizeof(GameConfig));
    cfg->ai_difficulty = -1;

    static struct option long_opts[] = {
        {"gui",     no_argument,       0, 'g'},
        {"cli",     no_argument,       0, 'c'},
        {"pvp",     no_argument,       0, 'p'},
        {"ai",      required_argument, 0, 'a'},
        {"file",    required_argument, 0, 'f'},
        {"replay",  optional_argument, 0, 'r'},
        {"dim",     required_argument, 0, 'd'},
        {"rows",    required_argument, 0, 'R'},
        {"cols",    required_argument, 0, 'C'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "gca:f:r::d:R:C:ph", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'g': cfg->display_mode = DISPLAY_GUI; break;
            case 'c': cfg->display_mode = DISPLAY_CONSOLE; break;
            case 'p': cfg->game_mode = MODE_PVP; break;
            case 'a':
                cfg->game_mode = MODE_AI;
                cfg->ai_difficulty = atoi(optarg);
                break;
            case 'f':
                cfg->game_mode = MODE_FILE;
                snprintf(cfg->file_path, sizeof(cfg->file_path), "%s", optarg);
                break;
            case 'r':
                cfg->game_mode = MODE_REPLAY;
                if (optarg) {
                    snprintf(cfg->replay_name, sizeof(cfg->replay_name), "%s", optarg);
                } else if (optind < argc && argv[optind][0] != '-') {
                    snprintf(cfg->replay_name, sizeof(cfg->replay_name), "%s", argv[optind++]);
                }
                break;
            case 'd':
                if (sscanf(optarg, "%dx%d", &cfg->rows, &cfg->cols) != 2) {
                    sscanf(optarg, "%d %d", &cfg->rows, &cfg->cols);
                }
                break;
            case 'R': cfg->rows = atoi(optarg); break;
            case 'C': cfg->cols = atoi(optarg); break;
            case 'h':
                config_print_usage(argv[0]);
                return 0;
            default:
                config_print_usage(argv[0]);
                return 0;
        }
    }

    int is_cli_launch = (argc > 1);

    if (is_cli_launch) {
        if (cfg->display_mode == 0) cfg->display_mode = DISPLAY_GUI;
        if (cfg->rows == 0 || cfg->cols == 0) {
            cfg->rows = 6;
            cfg->cols = 7;
        }
        if (cfg->game_mode == 0) {
            cfg->game_mode = MODE_AI;
            if (cfg->ai_difficulty == -1) cfg->ai_difficulty = 2;
        }
    } else {
        printf("Connect Four\n");

        int dm = prompt_display_mode();
        if (dm == -1) return 0;
        cfg->display_mode = (DisplayMode)dm;

        int gm = prompt_game_mode();
        if (gm == -1) return 0;
        cfg->game_mode = (GameMode)gm;

        if (cfg->game_mode != MODE_REPLAY) {
            if (!prompt_dimensions(&cfg->rows, &cfg->cols)) return 0;
        }
    }

    if (cfg->game_mode == MODE_REPLAY) {
        if (cfg->replay_name[0] == '\0') {
            if (!prompt_replay_selection(cfg->replay_name, sizeof(cfg->replay_name))) {
                return 0;
            }
        }
        if (!load_game_for_replay("database.dat", cfg->replay_name,
                                  &cfg->rows, &cfg->cols,
                                  &cfg->replay_moves, &cfg->replay_move_count)) {
            return 0;
        }
    } else if (cfg->game_mode == MODE_AI) {
        if (cfg->ai_difficulty < 0 || cfg->ai_difficulty > 2) {
            int diff = prompt_ai_difficulty();
            if (diff == -1) return 0;
            cfg->ai_difficulty = diff;
        }
    } else if (cfg->game_mode == MODE_FILE) {
        if (cfg->file_path[0] == '\0') {
            printf("File path for Player O moves: ");
            if (scanf("%255s", cfg->file_path) != 1) {
                fprintf(stderr, "error reading file path\n");
                return 0;
            }
            flush_input();
        }
        cfg->file_handle = fopen(cfg->file_path, "r");
        if (!cfg->file_handle) {
            fprintf(stderr, "error: cannot open '%s'\n", cfg->file_path);
            return 0;
        }
    }

    if (cfg->rows < MIN_DIM || cfg->rows > MAX_DIM ||
        cfg->cols < MIN_DIM || cfg->cols > MAX_DIM) {
        fprintf(stderr, "error: dimensions (%dx%d) must be between 4 and 11\n", cfg->rows, cfg->cols);
        return 0;
    }

    return 1;
}

int config_setup_players(const GameConfig *cfg,
                         Player players[2],
                         AIContext *ai_ctx,
                         ReplayContext *replay_ctx,
                         DisplayFn *display_fn) {
    MoveFn human_move_fn = human_move_cli;
    *display_fn = display_board;

    if (cfg->display_mode == DISPLAY_GUI) {
        if (!gui_init(cfg->rows, cfg->cols)) {
            fprintf(stderr, "warning: SDL2 init failed, using console\n");
        } else {
            *display_fn = gui_render_board;
            human_move_fn = human_move_gui;
        }
    }

    if (cfg->game_mode == MODE_PVP) {
        players[0].token = 'X';
        players[0].move  = human_move_fn;
        players[0].ctx   = &players[0].token;

        players[1].token = 'O';
        players[1].move  = human_move_fn;
        players[1].ctx   = &players[1].token;
    } else if (cfg->game_mode == MODE_AI) {
        players[0].token = 'X';
        players[0].move  = human_move_fn;
        players[0].ctx   = &players[0].token;

        ai_ctx->difficulty = cfg->ai_difficulty;
        players[1].token = 'O';
        players[1].move  = ai_move;
        players[1].ctx   = ai_ctx;
    } else if (cfg->game_mode == MODE_FILE) {
        players[0].token = 'X';
        players[0].move  = human_move_fn;
        players[0].ctx   = &players[0].token;

        players[1].token = 'O';
        players[1].move  = file_move;
        players[1].ctx   = cfg->file_handle;
    } else if (cfg->game_mode == MODE_REPLAY) {
        replay_ctx->moves         = cfg->replay_moves;
        replay_ctx->total_moves   = cfg->replay_move_count;
        replay_ctx->current_index = 0;

        players[0].token = 'X';
        players[0].move  = replay_move;
        players[0].ctx   = replay_ctx;

        players[1].token = 'O';
        players[1].move  = replay_move;
        players[1].ctx   = replay_ctx;
    }

    return 1;
}

void config_cleanup(GameConfig *cfg) {
    if (cfg->file_handle) {
        fclose(cfg->file_handle);
        cfg->file_handle = NULL;
    }
    if (cfg->replay_moves) {
        free(cfg->replay_moves);
        cfg->replay_moves = NULL;
    }
}
