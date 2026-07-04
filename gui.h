#ifndef GUI_H
#define GUI_H

#include "engine.h"

#define GUI_CELL_SIZE 90

int init_gui(int rows, int cols);
void shutdown_gui(void);
void display_board_gui(const GameState *st);
int GUI_Human_Move(const GameState *st, void *ctx);

#endif
