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
        printf("Save aborted: Game '%s' already exists. Older save preserved.\n", game_name);
        return 0;
    }
    
    FILE *db = fopen(db_filename, "a");
    if (!db) {
        fprintf(stderr, "Error: Could not open database file for writing.\n");
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
        fprintf(stderr, "Error: Database file not found.\n");
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
        printf("Error: Game '%s' not found in database.\n", target_name);
        return 0; 
    }

    if (fgets(line, sizeof(line), db) && strncmp(line, "Dimensions: ", 12) == 0) {
        if (sscanf(line + 12, "%d %d", out_rows, out_cols) != 2 ||
            *out_rows < 4 || *out_rows > MAX_DIM ||
            *out_cols < 4 || *out_cols > MAX_DIM) {
            fclose(db);
            fprintf(stderr, "Error: Corrupted database format (dimensions out of range).\n");
            return 0;
        }
    } else {
        fclose(db); 
        fprintf(stderr, "Error: Corrupted database format (Dimensions missing).\n");
        return 0; 
    }

    if (fgets(line, sizeof(line), db) && strncmp(line, "Moves: ", 7) == 0) {
        int max_moves = (*out_rows) * (*out_cols);
        *out_moves = malloc(max_moves * sizeof(int));
        if (!*out_moves) {
            fclose(db);
            fprintf(stderr, "Error: Memory allocation failed while loading replay.\n");
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
        fprintf(stderr, "Error: Corrupted database format (Moves missing).\n");
        return 0;
    }

    fclose(db);
    return 1;
}
