#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <stdlib.h>

#define MIN_DIM 4
#define MAX_DIM 11

#define MOVE_QUIT -1
#define MOVE_UNDO -2

typedef struct GameState GameState;
typedef int (*MoveFn)(const GameState *st, void *ctx);
typedef void (*DisplayFn)(const GameState *st);

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
int play_game(GameState *st, Player players[2], int *move_history, int *move_count, DisplayFn display);
int get_winning_line(const GameState *st, char token, int win_coords[4][2]);
int check_win(const GameState *st, char token);
int check_draw(const GameState *st);
int is_valid_move(const GameState *st, int c);
void undo_drop(GameState *st, int c);

#endif
