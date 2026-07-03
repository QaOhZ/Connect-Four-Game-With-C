#include "players.h"

int Human_Move(const GameState *st, void *ctx) {
    (void)ctx; 

    int col;
    int parsed_items;

    while (1) {
        printf("Enter a column (0 to %d): ", st->cols - 1);

        parsed_items = scanf("%d", &col);

        if (parsed_items == EOF) {
            printf("\nEOF detected: terminating input.\n");
            return -1;
        }

        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
        }

        if (parsed_items == 1) {
            if (col >= 0 && col < st->cols) {
                return col;
            } else {
                printf("Error: Column out of bounds. Please try again.\n");
            }
        } else {
            printf("Error: Invalid input. Please enter a valid number.\n");
        }
    }
}

int File_Move(const GameState *st, void *ctx) {
    FILE *file = (FILE *)ctx;
    
    if (!file) {
        printf("Fatal Error: File pointer is NULL.\n");
        return -1;
    }

    int col;
    int read_status = fscanf(file, "%d", &col);

    if (read_status == EOF) {
        printf("Notice: Reached End of File (EOF). The game is incomplete.\n");
        return -1;
    } else if (read_status != 1) {
        printf("Error: Corrupted or invalid data encountered in file.\n");
        return -1; 
    }

    if (col < 0 || col >= st->cols) {
        printf("Error: File provided out-of-bounds column (%d).\n", col);
        return -1; 
    }

    if (!is_valid_move(st, col)) {
        printf("Error: File provided full column (%d).\n", col);
        return -1;
    }

    printf("File player selects column %d\n", col);
    
    return col;
}

int get_best_minimax_move(GameState *st, int depth) {
    int best_col = -1;
    int best_score = -1000000;

    for (int c = 0; c < st->cols; c++) {
        if (is_valid_move(st, c)) {
            simulate_drop(st, c, AI_TOKEN);
            int score = minimax(st, depth - 1, -1000000, 1000000, 0);
            undo_drop(st, c);

            if (score > best_score) {
                best_score = score;
                best_col = c;
            }
        }
    }
    return best_col;
}

int AI_Move(const GameState *st, void *ctx) {
    AIContext *ai = (AIContext *)ctx;
    
    if (ai->difficulty == 0) {
        return random_move(st); 
    } 
    else if (ai->difficulty == 1) {
        return heuristic_move((GameState *)st, AI_TOKEN, HUMAN_TOKEN); 
    } 
    else {
        return get_best_minimax_move((GameState *)st, 6); 
    }
}

int minimax(GameState *st, int depth, int alpha, int beta, int is_maximizing) {
    if (check_win(st, AI_TOKEN)) return 10000;
    if (check_win(st, HUMAN_TOKEN)) return -10000;
    if (check_draw(st)) return 0;

    if (depth == 0) {
        return evaluate_board(st, AI_TOKEN); 
    }

    if (is_maximizing) {
        int max_eval = -999999;
        for (int c = 0; c < st->cols; c++) {
            if (is_valid_move(st, c)) {
                simulate_drop(st, c, AI_TOKEN);
                
                int eval = minimax(st, depth - 1, alpha, beta, 0);
                
                undo_drop(st, c);
                
                if (eval > max_eval) max_eval = eval;
                if (eval > alpha) alpha = eval;
                if (beta <= alpha) break;
            }
        }
        return max_eval;
    } else {
        int min_eval = 999999; 
        
        for (int c = 0; c < st->cols; c++) {
            if (is_valid_move(st, c)) {
                simulate_drop(st, c, HUMAN_TOKEN); 
                
                int eval = minimax(st, depth - 1, alpha, beta, 1);
                
                undo_drop(st, c); 
                
                if (eval < min_eval) min_eval = eval;
                
                if (eval < beta) beta = eval;
                
                if (beta <= alpha) break; 
            }
        }
        return min_eval;
    }
}

int evaluate_window(char window[4], char ai_token) {
    int score = 0;
    
    char opp_token = (ai_token == 'X') ? 'O' : 'X';
    
    int ai_pieces = 0;
    int opp_pieces = 0;
    int empty_spaces = 0;

    for (int i = 0; i < 4; i++) {
        if (window[i] == ai_token) {
            ai_pieces++;
        } else if (window[i] == opp_token) {
            opp_pieces++;
        } else if (window[i] == '.') {
            empty_spaces++;
        }
    }

    if (ai_pieces == 4) {
        score += 1000;
    } else if (ai_pieces == 3 && empty_spaces == 1) {
        score += 50;
    } else if (ai_pieces == 2 && empty_spaces == 2) {
        score += 10;
    }

    if (opp_pieces == 3 && empty_spaces == 1) {
        score -= 80;
    }

    return score;
}


int evaluate_board(const GameState *st, char ai_token) {
    int score = 0;
    int rows = st->rows;
    int cols = st->cols;
    char window[4];

    int center_col = cols / 2;
    int center_count = 0;
    for (int r = 0; r < rows; r++) {
        if (st->board[r * cols + center_col] == ai_token) {
            center_count++;
        }
    }
    score += center_count * 3; 

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c <= cols - 4; c++) {
            window[0] = st->board[r * cols + c];
            window[1] = st->board[r * cols + (c + 1)];
            window[2] = st->board[r * cols + (c + 2)];
            window[3] = st->board[r * cols + (c + 3)];
            
            score += evaluate_window(window, ai_token);
        }
    }

    for (int r = 0; r <= rows - 4; r++) {
        for (int c = 0; c < cols; c++) {
            window[0] = st->board[r * cols + c];
            window[1] = st->board[(r + 1) * cols + c];
            window[2] = st->board[(r + 2) * cols + c];
            window[3] = st->board[(r + 3) * cols + c];
            score += evaluate_window(window, ai_token);
        }
    }

    for (int r = 0; r <= rows - 4; r++) {
        for (int c = 0; c <= cols - 4; c++) {
            window[0] = st->board[r * cols + c];
            window[1] = st->board[(r + 1) * cols + (c + 1)];
            window[2] = st->board[(r + 2) * cols + (c + 2)];
            window[3] = st->board[(r + 3) * cols + (c + 3)];
            score += evaluate_window(window, ai_token);
        }
    }


    for (int r = 3; r < rows; r++) {
        for (int c = 0; c <= cols - 4; c++) {
            window[0] = st->board[r * cols + c];
            window[1] = st->board[(r - 1) * cols + (c + 1)];
            window[2] = st->board[(r - 2) * cols + (c + 2)];
            window[3] = st->board[(r - 3) * cols + (c + 3)];
            score += evaluate_window(window, ai_token);
        }
    }

    return score;
}

void simulate_drop(GameState *st, int c, char token) {
    for (int r = st->rows - 1; r >= 0; r--) {
        int index = r * st->cols + c;
        if (st->board[index] == '.') {
            st->board[index] = token;
            break;
        }
    }
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

int random_move(const GameState *st) {
    int valid_cols[MAX_DIM];
    int valid_count = 0;

    for (int c = 0; c < st->cols; c++) {
        if (is_valid_move(st, c)) {
            valid_cols[valid_count] = c;
            valid_count++;
        }
    }

    if (valid_count == 0) return -1;
    return valid_cols[rand() % valid_count];
}

int heuristic_move(GameState *st, char ai_token, char human_token) {
    for (int c = 0; c < st->cols; c++) {
        if (is_valid_move(st, c)) {
            simulate_drop(st, c, ai_token);
            if (check_win(st, ai_token)) {
                undo_drop(st, c);
                return c;
            }
            undo_drop(st, c);
        }
    }
    
    for (int c = 0; c < st->cols; c++) {
        if (is_valid_move(st, c)) {
            simulate_drop(st, c, human_token);
            if (check_win(st, human_token)) {
                undo_drop(st, c);
                return c;
            }
            undo_drop(st, c);
        }
    }
    
    int best_col = -1;
    int max_eval = -999999;
    
    for (int c = 0; c < st->cols; c++) {
        if (is_valid_move(st, c)) {
            simulate_drop(st, c, ai_token);
            
            int eval = evaluate_board(st, ai_token);
            undo_drop(st, c);
            
            if (eval > max_eval) {
                max_eval = eval;
                best_col = c;
            }
        }
    }
    
    if (best_col == -1) {
        for (int c = 0; c < st->cols; c++) {
            if (is_valid_move(st, c)) return c;
        }
    }
    
    return best_col;
}

int Replay_Move(const GameState *st, void *ctx) {
    ReplayContext *replay = (ReplayContext *)ctx;
    
    if (!replay || !replay->moves) {
        printf("Error: Invalid replay context.\n");
        return -1;
    }
    
    if (replay->current_index >= replay->total_moves) {
        printf("Replay complete: no more moves in history.\n");
        return -1;
    }
    
    int col = replay->moves[replay->current_index++];

    if (!is_valid_move(st, col)) {
        printf("Error: Saved game data contains an invalid move (column %d). Stopping replay.\n", col);
        return -1;
    }

    printf("\n[Replay] Player %c selects column %d.\n", 
           (replay->current_index % 2 != 0) ? 'X' : 'O', col);
           
    printf("Press [Enter] to see the next move...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    return col;
}
