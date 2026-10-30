#include "players.h"
#include <SDL2/SDL.h>
#include "gui.h"

static void get_center_out_order(int cols, int order[MAX_DIM]) {
    int mid = cols / 2;
    int count = 0;

    order[count++] = mid;
    for (int i = 1; i <= cols / 2; i++) {
        if (mid - i >= 0) {
            order[count++] = mid - i;
        }
        if (mid + i < cols) {
            order[count++] = mid + i;
        }
    }
}

static int get_dynamic_depth(int cols) {
    if (cols <= 7) {
        return 6;
    } else if (cols <= 9) {
        return 5;
    } else {
        return 4;
    }
}

int human_move_cli(const GameState *st, void *ctx) {
    (void)ctx; 

    char line[64];
    while (1) {
        printf("Column (0-%d, 'u' undo, 'q' quit): ", st->cols - 1);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            return MOVE_QUIT;
        }

        char *p = line;
        while (*p == ' ' || *p == '\t') p++;

        if (*p == 'u' || *p == 'U') {
            return MOVE_UNDO;
        }
        if (*p == 'q' || *p == 'Q') {
            return MOVE_QUIT;
        }

        int col;
        if (sscanf(p, "%d", &col) == 1) {
            if (col == MOVE_UNDO) {
                return MOVE_UNDO;
            }
            if (col >= 0 && col < st->cols) {
                return col;
            } else {
                printf("Column out of bounds.\n");
            }
        } else {
            printf("Invalid input. Enter column number, 'u', or 'q'.\n");
        }
    }
}

int file_move(const GameState *st, void *ctx) {
    FILE *file = (FILE *)ctx;
    
    if (!file) {
        fprintf(stderr, "error: file pointer is null\n");
        return -1;
    }

    int col;
    int read_status = fscanf(file, "%d", &col);

    if (read_status == EOF) {
        printf("End of file reached.\n");
        return -1;
    } else if (read_status != 1) {
        fprintf(stderr, "error: invalid data in file\n");
        return -1; 
    }

    if (col < 0 || col >= st->cols) {
        fprintf(stderr, "error: column %d out of bounds\n", col);
        return -1; 
    }

    if (!is_valid_move(st, col)) {
        fprintf(stderr, "error: column %d is full\n", col);
        return -1; 
    }

    printf("File move: column %d\n", col);
    
    return col;
}

int get_best_minimax_move(GameState *st, int depth) {
    int best_col = -1;
    int best_score = -1000000;

    int order[MAX_DIM];
    get_center_out_order(st->cols, order);

    for (int i = 0; i < st->cols; i++) {
        int c = order[i];
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

    if (best_col == -1) {
        for (int i = 0; i < st->cols; i++) {
            int c = order[i];
            if (is_valid_move(st, c)) return c;
        }
    }

    return best_col;
}

int ai_move(const GameState *st, void *ctx) {
    AIContext *ai = (AIContext *)ctx;
    
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        gui_indicate_ai_thinking(st);
    } else {
        printf("AI thinking...\n");
        SDL_Delay(250);
    }

    if (ai->difficulty == 0) {
        return random_move(st); 
    } 
    else if (ai->difficulty == 1) {
        return heuristic_move((GameState *)st, AI_TOKEN, HUMAN_TOKEN); 
    } 
    else {
        int depth = get_dynamic_depth(st->cols);
        return get_best_minimax_move((GameState *)st, depth); 
    }
}

int minimax(GameState *st, int depth, int alpha, int beta, int is_maximizing) {
    if (check_win(st, AI_TOKEN)) return 10000;
    if (check_win(st, HUMAN_TOKEN)) return -10000;
    if (check_draw(st)) return 0;

    if (depth == 0) {
        return evaluate_board(st, AI_TOKEN); 
    }

    int order[MAX_DIM];
    get_center_out_order(st->cols, order);

    if (is_maximizing) {
        int max_eval = -999999;
        for (int i = 0; i < st->cols; i++) {
            int c = order[i];
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
        
        for (int i = 0; i < st->cols; i++) {
            int c = order[i];
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
    int order[MAX_DIM];
    get_center_out_order(st->cols, order);

    for (int i = 0; i < st->cols; i++) {
        int c = order[i];
        if (is_valid_move(st, c)) {
            simulate_drop(st, c, ai_token);
            if (check_win(st, ai_token)) {
                undo_drop(st, c);
                return c;
            }
            undo_drop(st, c);
        }
    }
    
    for (int i = 0; i < st->cols; i++) {
        int c = order[i];
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
    
    for (int i = 0; i < st->cols; i++) {
        int c = order[i];
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
        for (int i = 0; i < st->cols; i++) {
            int c = order[i];
            if (is_valid_move(st, c)) return c;
        }
    }
    
    return best_col;
}

int replay_move(const GameState *st, void *ctx) {
    ReplayContext *replay = (ReplayContext *)ctx;
    
    if (!replay || !replay->moves) {
        fprintf(stderr, "error: invalid replay context\n");
        return -1;
    }
    
    if (replay->current_index >= replay->total_moves) {
        printf("Replay finished.\n");
        return -1;
    }
    
    int col = replay->moves[replay->current_index++];

    if (!is_valid_move(st, col)) {
        fprintf(stderr, "error: invalid move in replay data (column %d)\n", col);
        return -1;
    }

    printf("[Replay] Player %c -> column %d\n", 
           (replay->current_index % 2 != 0) ? 'X' : 'O', col);
           
    printf("Press Enter for next move...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    return col;
}
