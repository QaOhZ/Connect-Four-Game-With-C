#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "engine.h"
#include "players.h"
#include "database.h"
#include "gui.h"

void flush_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int get_game_mode() {
    int mode;
    int parsed;
    
    printf("\n========================================\n");
    printf("       CONNECT FOUR: MODE SELECTION     \n");
    printf("========================================\n\n");
    
    printf("Select Game Mode:\n");
    printf("  1 - Human vs Human\n");
    printf("  2 - Human vs AI\n");
    printf("  3 - File Input Mode (player from file)\n");
    printf("  4 - Replay Mode (watch saved game)\n");
    printf("\nEnter mode (1-4): ");
    
    while (1) {
        parsed = scanf("%d", &mode);
        flush_input();
        
        if (parsed == EOF) {
            return -1;
        }
        
        if (parsed == 1 && mode >= 1 && mode <= 4) {
            return mode;
        }
        printf("Error: Please enter a number between 1 and 4: ");
    }
}

int get_display_mode() {
    int mode;
    int parsed;

    printf("\nSelect Display Mode:\n");
    printf("  1 - Console (text)\n");
    printf("  2 - Graphical (SDL2 window)\n");
    printf("\nEnter choice (1-2): ");

    while (1) {
        parsed = scanf("%d", &mode);
        flush_input();

        if (parsed == EOF) {
            return -1;
        }

        if (parsed == 1 && (mode == 1 || mode == 2)) {
            return mode;
        }
        printf("Error: Please enter 1 or 2: ");
    }
}

int main(int argc, char *argv[]) {
    (void)argc; 
    (void)argv;
    
    srand(time(NULL));

    printf("========================================\n");
    printf("       CONNECT FOUR: SYSTEMS ENGINE     \n");
    printf("========================================\n\n");

    int rows = 0, cols = 0;
    int parsed;

    while (1) {
        printf("Enter board dimensions (Rows Cols) [Limits: 4-11]: ");
        parsed = scanf("%d %d", &rows, &cols);
        flush_input();

        if (parsed == EOF) {
            return 0;
        }
        
        if (parsed == 2 && rows >= MIN_DIM && rows <= MAX_DIM && cols >= MIN_DIM && cols <= MAX_DIM) {
            break;
        }
        printf("Error: Invalid dimensions. Please enter two integers between 4 and 11.\n");
    }

    GameState *st = init_game_state(rows, cols);
    if (!st) {
        fprintf(stderr, "Fatal Error: Memory allocation failed. System abort.\n");
        return 1;
    }

    int display_mode = get_display_mode();
    if (display_mode == -1) {
        printf("EOF detected. Shutting down.\n");
        free_game_state(st);
        return 0;
    }

    DisplayFn display_fn = display_board;
    MoveFn human_move_fn = Human_Move;

    if (display_mode == 2) {
        if (!init_gui(rows, cols)) {
            fprintf(stderr, "Could not start the graphical window - continuing in console mode.\n");
            display_mode = 1;
        } else {
            display_fn = display_board_gui;
            human_move_fn = GUI_Human_Move;
        }
    }

    int game_mode = get_game_mode();
    
    if (game_mode == -1) {
        printf("EOF detected. Shutting down.\n");
        if (display_mode == 2) shutdown_gui();
        free_game_state(st);
        return 0;
    }

    Player players[2];
    int difficulty = 0;
    FILE *file_ctx = NULL;
    ReplayContext replay_ctx;
    int *replay_moves_ptr = NULL;
    AIContext ai_ctx;
    
    if (game_mode == 1) {
        players[0].move = human_move_fn;
        players[0].ctx = NULL;
        players[0].token = 'X';
        
        players[1].move = human_move_fn;
        players[1].ctx = NULL;
        players[1].token = 'O';
        
    } else if (game_mode == 2) {
        players[0].move = human_move_fn;
        players[0].ctx = NULL;
        players[0].token = 'X';

        while (1) {
            printf("Select AI Difficulty (0=Easy, 1=Medium, 2=Hard): ");
            parsed = scanf("%d", &difficulty);
            flush_input();
            
            if (parsed == EOF) {
                if (display_mode == 2) shutdown_gui();
                free_game_state(st);
                return 0;
            }
            
            if (parsed == 1 && difficulty >= 0 && difficulty <= 2) {
                break;
            }
            printf("Error: Invalid difficulty tier.\n");
        }
        
        ai_ctx.difficulty = difficulty;
        
        players[1].move = AI_Move;
        players[1].ctx = &ai_ctx;
        players[1].token = 'O';
        
    } else if (game_mode == 3) {
        players[0].move = human_move_fn;
        players[0].ctx = NULL;
        players[0].token = 'X';
        
        char filename[100];
        printf("Enter file path for Player O moves: ");
        if (scanf("%99s", filename) != 1) {
            printf("Error reading file path.\n");
            if (display_mode == 2) shutdown_gui();
            free_game_state(st);
            return 1;
        }
        flush_input();
        
        file_ctx = fopen(filename, "r");
        if (!file_ctx) {
            printf("Error: Could not open file '%s'.\n", filename);
            if (display_mode == 2) shutdown_gui();
            free_game_state(st);
            return 1;
        }
        
        players[1].move = File_Move;
        players[1].ctx = file_ctx;
        players[1].token = 'O';
        
    } else if (game_mode == 4) {
        char game_name[50];
        printf("Enter the name of the saved game to replay: ");
        if (scanf("%49s", game_name) != 1) {
            printf("Error reading game name.\n");
            if (display_mode == 2) shutdown_gui();
            free_game_state(st);
            return 1;
        }
        flush_input();
        
        int out_rows, out_cols, out_moves;
        int *out_move_history = NULL;
        
        if (!load_game_for_replay("database.dat", game_name, &out_rows, &out_cols, &out_move_history, &out_moves)) {
            printf("Failed to load game for replay.\n");
            if (display_mode == 2) shutdown_gui();
            free_game_state(st);
            return 1;
        }
        
        replay_moves_ptr = out_move_history;
        
        rows = out_rows;
        cols = out_cols;
        
        free_game_state(st);
        st = init_game_state(rows, cols);
        if (!st) {
            fprintf(stderr, "Failed to initialize replay board.\n");
            if (display_mode == 2) shutdown_gui();
            free(out_move_history);
            return 1;
        }
        
        replay_ctx.moves = out_move_history;
        replay_ctx.total_moves = out_moves;
        replay_ctx.current_index = 0;
        
        players[0].move = Replay_Move;
        players[0].ctx = &replay_ctx;
        players[0].token = 'X';
        
        players[1].move = Replay_Move;
        players[1].ctx = &replay_ctx;
        players[1].token = 'O';
    }
    
    int max_moves = rows * cols;
    int *move_history = malloc(max_moves * sizeof(int));
    if (!move_history) {
        fprintf(stderr, "Fatal Error: Memory allocation failed for move history. System abort.\n");
        if (display_mode == 2) shutdown_gui();
        free_game_state(st);
        return 1;
    }
    int move_count = 0;

    printf("\nInitialization complete. Engaging core engine...\n\n");

    int winner = play_game(st, players, move_history, &move_count, display_fn);

    printf("\n========== MATCH RESULTS ==========\n");
    if (winner == 'X') {
        printf("Player X wins!\n");
    } else if (winner == 'O') {
        printf("Player O wins!\n");
    } else {
        printf("Draw match!\n");
    }
    printf("Total moves played: %d\n", move_count);
    printf("=================================\n");

    if (game_mode == 3 && file_ctx) {
        fclose(file_ctx);
    }
    
    if (game_mode == 4 && replay_moves_ptr) {
        free(replay_moves_ptr);
    }

    if (game_mode != 4 && move_count > 0) {
        printf("\nWould you like to save this game? (y/n): ");
        char save_choice;
        int parsed = scanf(" %c", &save_choice);
        flush_input();

        if (parsed != 1) {
            printf("Error reading save choice.\n");
        } else if (save_choice == 'y' || save_choice == 'Y') {
            char game_name[50];
            printf("Enter a name for this save file (no spaces, max 49 chars): ");
            
            if (scanf("%49s", game_name) != 1) {
                printf("Error reading save name.\n");
            } else {
                flush_input();
                
                if (save_game("database.dat", game_name, rows, cols, move_history, move_count)) {
                    printf("Success: Game state securely written to database.dat\n");
                }
            }
        }
    }

    if (display_mode == 2) {
        shutdown_gui();
    }

    free(move_history);
    free_game_state(st);
    
    printf("\nMemory released. System shutting down cleanly.\n");
    return 0;
}
