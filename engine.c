#include "engine.h"

GameState* init_game_state(int r, int c) {
    if (r < MIN_DIM || r > MAX_DIM || c < MIN_DIM || c > MAX_DIM) {
        fprintf(stderr, "error: invalid dimensions (%dx%d), must be 4-11\n", r, c);
        return NULL; 
    }

    GameState *st = malloc(sizeof(GameState));
    if (!st) return NULL;

    st->rows = r;
    st->cols = c;
    
    st->board = malloc(r * c * sizeof(char));
    if (!st->board) {
        free(st);
        return NULL;
    }
    
    for (int row = 0; row < r; row++) {
        for (int col = 0; col < c; col++) {
            st->board[row * st->cols + col] = '.'; 
        }
    }
    
    return st;
}

void free_game_state(GameState *st) {
    if (st) {
        free(st->board);
        free(st);
    }
}

void display_board(const GameState *st) {
    printf("\n   ");
    for (int c = 0; c < st->cols; c++) {
        printf("%2d", c);
    }
    printf("\n");
    
    printf("  +");
    for (int c = 0; c < st->cols; c++) {
        printf("--");
    }
    printf("-+\n");
    
    for (int row = 0; row < st->rows; row++) {
        printf("  |");
        for (int col = 0; col < st->cols; col++) {
            printf(" %c", st->board[row * st->cols + col]);
        }
        printf(" |\n");
    }
    
    printf("  +");
    for (int c = 0; c < st->cols; c++) {
        printf("--");
    }
    printf("-+\n");
    printf("\n");
}

int play_game(GameState *st, Player players[2], int *move_history, int *move_count, DisplayFn display) {
    int current_turn = 0;
    int game_over = 0;
    int winner = 0;

    display(st);

    while (!game_over) {
        Player *current_player = &players[current_turn];

        int col = current_player->move(st, current_player->ctx);
        
        if (col == MOVE_QUIT) {
            printf("\nPlayer %c quit.\n", current_player->token);
            winner = (current_turn == 0) ? players[1].token : players[0].token;
            game_over = 1;
            break;
        }

        if (col == MOVE_UNDO) {
            if (*move_count > 0) {
                int steps = (players[0].move != players[1].move && *move_count >= 2) ? 2 : 1;
                for (int s = 0; s < steps; s++) {
                    (*move_count)--;
                    int last_col = move_history[*move_count];
                    undo_drop(st, last_col);
                    current_turn = 1 - current_turn;
                }
                printf("\nUndid move.\n");
                display(st);
            } else {
                printf("\nNothing to undo.\n");
            }
            continue;
        }

        if (!is_valid_move(st, col)) {
            printf("Column %d is full or out of bounds.\n", col);
            continue;
        }

        int placed = 0;
        for (int row = st->rows - 1; row >= 0; row--) {
            int index = row * st->cols + col;
            
            if (st->board[index] == '.') {
                st->board[index] = current_player->token;
                placed = 1;
                break;
            }
        }

        if (placed) {
            move_history[*move_count] = col;
            (*move_count)++;
            
            display(st);
            
            if (check_win(st, current_player->token)) {
                winner = current_player->token;
                printf("Player %c wins!\n", current_player->token);
                game_over = 1;
                break;
            }
            
            if (check_draw(st)) {
                printf("Draw: board full.\n");
                game_over = 1;
                break;
            }
            
            current_turn = 1 - current_turn; 
        }
    }
    
    return winner;
}

int get_winning_line(const GameState *st, char token, int win_coords[4][2]) {
    int rows = st->rows;
    int cols = st->cols;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c <= cols - 4; c++) {
            if (st->board[r * cols + c] == token &&
                st->board[r * cols + (c + 1)] == token &&
                st->board[r * cols + (c + 2)] == token &&
                st->board[r * cols + (c + 3)] == token) {
                if (win_coords) {
                    for (int i = 0; i < 4; i++) {
                        win_coords[i][0] = r;
                        win_coords[i][1] = c + i;
                    }
                }
                return 1;
            }
        }
    }

    for (int r = 0; r <= rows - 4; r++) {
        for (int c = 0; c < cols; c++) {
            if (st->board[r * cols + c] == token &&
                st->board[(r + 1) * cols + c] == token &&
                st->board[(r + 2) * cols + c] == token &&
                st->board[(r + 3) * cols + c] == token) {
                if (win_coords) {
                    for (int i = 0; i < 4; i++) {
                        win_coords[i][0] = r + i;
                        win_coords[i][1] = c;
                    }
                }
                return 1;
            }
        }
    }

    for (int r = 0; r <= rows - 4; r++) {
        for (int c = 0; c <= cols - 4; c++) {
            if (st->board[r * cols + c] == token &&
                st->board[(r + 1) * cols + (c + 1)] == token &&
                st->board[(r + 2) * cols + (c + 2)] == token &&
                st->board[(r + 3) * cols + (c + 3)] == token) {
                if (win_coords) {
                    for (int i = 0; i < 4; i++) {
                        win_coords[i][0] = r + i;
                        win_coords[i][1] = c + i;
                    }
                }
                return 1;
            }
        }
    }

    for (int r = 3; r < rows; r++) {
        for (int c = 0; c <= cols - 4; c++) {
            if (st->board[r * cols + c] == token &&
                st->board[(r - 1) * cols + (c + 1)] == token &&
                st->board[(r - 2) * cols + (c + 2)] == token &&
                st->board[(r - 3) * cols + (c + 3)] == token) {
                if (win_coords) {
                    for (int i = 0; i < 4; i++) {
                        win_coords[i][0] = r - i;
                        win_coords[i][1] = c + i;
                    }
                }
                return 1;
            }
        }
    }

    return 0;
}

int check_win(const GameState *st, char token) {
    return get_winning_line(st, token, NULL);
}

int is_valid_move(const GameState *st, int c) {
    if (c < 0 || c >= st->cols) return 0;
    return st->board[c] == '.';
}

int check_draw(const GameState *st) {
    for (int c = 0; c < st->cols; c++) {
        if (st->board[c] == '.') {
            return 0;
        }
    }
    return 1;
}

void undo_drop(GameState *st, int c) {
    for (int r = 0; r < st->rows; r++) {
        int index = r * st->cols + c;
        if (st->board[index] != '.') {
            st->board[index] = '.';
            break;
        }
    }
}
