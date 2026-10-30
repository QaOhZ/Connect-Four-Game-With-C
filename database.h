#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[50];
    int rows;
    int cols;
    int move_count;
} SavedGameInfo;

int save_game(const char *db_filename, const char *game_name, int rows, int cols, const int *moves, int move_count);
int load_game_for_replay(const char *db_filename, const char *target_name, int *out_rows, int *out_cols, int **out_moves, int *out_move_count);
int list_saved_games(const char *db_filename, SavedGameInfo *out_list, int max_entries);

#endif
