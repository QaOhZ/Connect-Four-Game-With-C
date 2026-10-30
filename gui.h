#ifndef GUI_H
#define GUI_H

#include "engine.h"

#define GUI_CELL_SIZE 90

int  gui_init(int rows, int cols);
void gui_shutdown(void);
void gui_render_board(const GameState *st);
int  human_move_gui(const GameState *st, void *ctx);
void gui_show_game_over(const GameState *st, int winner);
void gui_indicate_ai_thinking(const GameState *st);
int  gui_save_screenshot(const char *filename);
int  gui_render_and_save_screenshot(const GameState *st, int hover_col, char hover_token, int winner_token, const char *filename);
void gui_render_state(const GameState *st, int hover_col, char hover_token);

#endif
