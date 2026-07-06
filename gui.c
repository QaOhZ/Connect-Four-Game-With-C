#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "gui.h"

#define TOP_MARGIN_ROWS 1
#define PIECE_RADIUS ((GUI_CELL_SIZE / 2) - 6)

static SDL_Window   *g_window   = NULL;
static SDL_Renderer *g_renderer = NULL;

int init_gui(int rows, int cols) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    int width  = cols * GUI_CELL_SIZE;
    int height = (rows + TOP_MARGIN_ROWS) * GUI_CELL_SIZE;

    g_window = SDL_CreateWindow("Connect Four",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 width, height, SDL_WINDOW_SHOWN);
    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (!g_renderer) {
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!g_renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        g_window = NULL;
        SDL_Quit();
        return 0;
    }

    return 1;
}

void shutdown_gui(void) {
    if (g_renderer) { SDL_DestroyRenderer(g_renderer); g_renderer = NULL; }
    if (g_window)   { SDL_DestroyWindow(g_window);     g_window   = NULL; }
    SDL_Quit();
}

void display_board_gui(const GameState *st) {
    SDL_SetRenderDrawColor(g_renderer, 30, 60, 200, 255);
    SDL_RenderClear(g_renderer);

    for (int r = 0; r < st->rows; r++) {
        for (int c = 0; c < st->cols; c++) {
            int cx = c * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;
            int cy = (r + TOP_MARGIN_ROWS) * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;

            char cell = st->board[r * st->cols + c];
            Uint8 red, green, blue;
            if (cell == 'X') {
                red = 220; green = 30;  blue = 30;
            } else if (cell == 'O') {
                red = 240; green = 200; blue = 20;
            } else {
                red = 240; green = 240; blue = 245;
            }

            filledCircleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS, red, green, blue, 255);
            circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS, 10, 20, 90, 255);
        }
    }

    SDL_RenderPresent(g_renderer);
}

int GUI_Human_Move(const GameState *st, void *ctx) {
    (void)ctx;
    SDL_Event event;

    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return -1;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int col = event.button.x / GUI_CELL_SIZE;
                if (is_valid_move(st, col)) {
                    return col;
                }
                printf("Column %d is out of bounds or full - click a different column.\n", col);
            }
        }
        SDL_Delay(16);
    }
}
