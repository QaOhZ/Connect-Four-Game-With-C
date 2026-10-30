#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "database.h"
#include "gui.h"

static void handle_save_prompt(int rows, int cols, const int *history, int count) {
    if (count <= 0) return;

    printf("\nSave game? (y/n): ");
    char choice;
    if (scanf(" %c", &choice) == 1 && (choice == 'y' || choice == 'Y')) {
        char name[50];
        printf("Save name: ");
        if (scanf("%49s", name) == 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);

            if (save_game("database.dat", name, rows, cols, history, count)) {
                printf("Saved to database.dat\n");
            }
        }
    }
}

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));

    GameConfig cfg;
    if (!config_load(&cfg, argc, argv)) {
        return 0;
    }

    GameState *st = init_game_state(cfg.rows, cfg.cols);
    if (!st) {
        fprintf(stderr, "error: out of memory\n");
        config_cleanup(&cfg);
        return 1;
    }

    AIContext ai_ctx;
    ReplayContext replay_ctx;
    Player players[2];
    DisplayFn display_fn = NULL;
    config_setup_players(&cfg, players, &ai_ctx, &replay_ctx, &display_fn);

    int *move_history = malloc(cfg.rows * cfg.cols * sizeof(int));
    if (!move_history) {
        fprintf(stderr, "error: out of memory\n");
        if (cfg.display_mode == DISPLAY_GUI) gui_shutdown();
        free_game_state(st);
        config_cleanup(&cfg);
        return 1;
    }
    int move_count = 0;

    int winner = play_game(st, players, move_history, &move_count, display_fn);

    if (cfg.display_mode == DISPLAY_GUI) {
        gui_show_game_over(st, winner);
        gui_shutdown();
    }

    printf("\nGame Over: %s\n", (winner == 'X') ? "Player X wins!" :
                                (winner == 'O') ? "Player O wins!" : "Draw!");
    printf("Moves played: %d\n", move_count);

    if (cfg.game_mode != MODE_REPLAY) {
        handle_save_prompt(cfg.rows, cfg.cols, move_history, move_count);
    }

    free(move_history);
    free_game_state(st);
    config_cleanup(&cfg);
    return 0;
}
