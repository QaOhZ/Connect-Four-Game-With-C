#ifndef PLAYERS_H
#define PLAYERS_H

#include "engine.h"

#define AI_TOKEN 'O'
#define HUMAN_TOKEN 'X'

typedef struct {
    int difficulty;
} AIContext;

int Human_Move(const GameState *st, void *ctx);
int File_Move(const GameState *st, void *ctx);
int AI_Move(const GameState *st, void *ctx);

int minimax(GameState *st, int depth, int alpha, int beta, int is_maximizing);
int get_best_minimax_move(GameState *st, int depth);
int evaluate_board(const GameState *st, char ai_token);
int evaluate_window(char window[4], char ai_token);

void simulate_drop(GameState *st, int c, char token);
void undo_drop(GameState *st, int c);

int random_move(const GameState *st);
int heuristic_move(GameState *st, char ai_token, char human_token);

typedef struct {
    int *moves;
    int total_moves;
    int current_index;
} ReplayContext;

int Replay_Move(const GameState *st, void *ctx);

#endif
