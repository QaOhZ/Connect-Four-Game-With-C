#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>

#define MIN_DIM 4
#define MAX_DIM 11

typedef struct GameState GameState;
typedef int (*MoveFn)(const GameState *st, void *ctx);

struct GameState {
    int rows;
    int cols;
    char *board;
};

typedef struct {
    MoveFn move;
    void *ctx; 
    char token;
} Player;

GameState* init_game_state(int r, int c);
void free_game_state(GameState *st);
void display_board(const GameState *st);
int play_game(GameState *st, Player players[2], int *move_history, int *move_count);
int check_win(const GameState *st, char token);
int check_draw(const GameState *st);
int is_valid_move(const GameState *st, int c);

#endif
