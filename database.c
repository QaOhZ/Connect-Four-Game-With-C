#include "database.h"
#include "engine.h"

int save_game(const char *db_filename, const char *game_name, int rows, int cols, const int *moves, int move_count) {
    FILE *check_db = fopen(db_filename, "r");
    int duplicate_found = 0;
    
    if (check_db) {
        char line[MAX_DIM * MAX_DIM * 4 + 32];
        while (fgets(line, sizeof(line), check_db)) {
            if (strncmp(line, "Name: ", 6) == 0) {
                line[strcspn(line, "\n")] = 0;
                if (strcmp(line + 6, game_name) == 0) {
                    duplicate_found = 1;
                    break;
                }
            }
        }
        fclose(check_db);
    }
    
    if (duplicate_found) {
        fprintf(stderr, "error: save '%s' already exists\n", game_name);
        return 0;
    }
    
    FILE *db = fopen(db_filename, "a");
    if (!db) {
        fprintf(stderr, "error: cannot open %s for writing\n", db_filename);
        return 0;
    }

    fprintf(db, "[GAME_START]\n");
    fprintf(db, "Name: %s\n", game_name);
    fprintf(db, "Dimensions: %d %d\n", rows, cols);
    
    fprintf(db, "Moves: ");
    for (int i = 0; i < move_count; i++) {
        fprintf(db, "%d ", moves[i]);
    }
    fprintf(db, "\n");
    
    fprintf(db, "[GAME_END]\n\n");
    
    fclose(db);
    return 1;
}

int load_game_for_replay(const char *db_filename, const char *target_name, int *out_rows, int *out_cols, int **out_moves, int *out_move_count) {
    FILE *db = fopen(db_filename, "r");
    if (!db) {
        fprintf(stderr, "error: cannot open database '%s'\n", db_filename);
        return 0;
    }

    #define MAX_LINE_BUFFER (MAX_DIM * MAX_DIM * 4 + 32)
    char line[MAX_LINE_BUFFER];
    int found = 0;

    while (fgets(line, sizeof(line), db)) {
        if (strncmp(line, "Name: ", 6) == 0) {
            line[strcspn(line, "\n")] = 0;
            
            if (strcmp(line + 6, target_name) == 0) {
                found = 1;
                break;
            }
        }
    }

    if (!found) {
        fclose(db);
        fprintf(stderr, "error: game '%s' not found\n", target_name);
        return 0;
    }

    if (fgets(line, sizeof(line), db) && strncmp(line, "Dimensions: ", 12) == 0) {
        if (sscanf(line + 12, "%d %d", out_rows, out_cols) != 2 ||
            *out_rows < 4 || *out_rows > MAX_DIM ||
            *out_cols < 4 || *out_cols > MAX_DIM) {
            fclose(db);
            fprintf(stderr, "error: corrupted database (invalid dimensions)\n");
            return 0;
        }
    } else {
        fclose(db);
        fprintf(stderr, "error: corrupted database (missing dimensions)\n");
        return 0;
    }

    if (fgets(line, sizeof(line), db) && strncmp(line, "Moves: ", 7) == 0) {
        int max_moves = (*out_rows) * (*out_cols);
        *out_moves = malloc(max_moves * sizeof(int));
        if (!*out_moves) {
            fclose(db);
            fprintf(stderr, "error: memory allocation failed\n");
            return 0;
        }
        *out_move_count = 0;

        char *token = strtok(line + 7, " \n");
        while (token != NULL && *out_move_count < max_moves) {
            (*out_moves)[(*out_move_count)++] = atoi(token);
            token = strtok(NULL, " \n");
        }
    } else {
        fclose(db);
        fprintf(stderr, "error: corrupted database (missing moves)\n");
        return 0;
    }

    fclose(db);
    return 1;
}

int list_saved_games(const char *db_filename, SavedGameInfo *out_list, int max_entries) {
    FILE *db = fopen(db_filename, "r");
    if (!db) {
        printf("No saved games found in '%s'.\n", db_filename);
        return 0;
    }

    char line[MAX_DIM * MAX_DIM * 4 + 32];
    int count = 0;
    char current_name[50] = {0};
    int current_rows = 0, current_cols = 0;
    int in_game = 0;

    while (fgets(line, sizeof(line), db) && count < max_entries) {
        if (strncmp(line, "[GAME_START]", 12) == 0) {
            in_game = 1;
            current_name[0] = '\0';
            current_rows = 0;
            current_cols = 0;
        } else if (in_game && strncmp(line, "Name: ", 6) == 0) {
            line[strcspn(line, "\r\n")] = '\0';
            snprintf(current_name, sizeof(current_name), "%.*s", (int)(sizeof(current_name) - 1), line + 6);
        } else if (in_game && strncmp(line, "Dimensions: ", 12) == 0) {
            sscanf(line + 12, "%d %d", &current_rows, &current_cols);
        } else if (in_game && strncmp(line, "Moves: ", 7) == 0) {
            int moves = 0;
            char *p = line + 7;
            while (*p) {
                while (*p == ' ' || *p == '\t') p++;
                if (*p >= '0' && *p <= '9') {
                    moves++;
                    while (*p >= '0' && *p <= '9') p++;
                } else {
                    p++;
                }
            }
            if (out_list && count < max_entries) {
                snprintf(out_list[count].name, sizeof(out_list[count].name), "%s", current_name);
                out_list[count].rows = current_rows;
                out_list[count].cols = current_cols;
                out_list[count].move_count = moves;
                count++;
            }
            in_game = 0;
        }
    }

    fclose(db);

    if (count > 0) {
        printf("\nSaved games:\n");
        for (int i = 0; i < count; i++) {
            printf("  [%d] %-16s (%dx%d, %d moves)\n",
                   i + 1,
                   out_list[i].name,
                   out_list[i].rows,
                   out_list[i].cols,
                   out_list[i].move_count);
        }
        printf("\n");
    } else {
        printf("\nNo saved games available.\n");
    }

    return count;
}
