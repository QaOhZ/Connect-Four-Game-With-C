#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <string.h>
#include "gui.h"

#define TOP_MARGIN_ROWS 1
#define PIECE_RADIUS ((GUI_CELL_SIZE / 2) - 6)

typedef struct {
    Uint8 r, g, b, a;
} ColorRGBA;

/* Tabletop Connect Four Color Palette */
static const ColorRGBA COLOR_BANNER_BG     = {14, 28, 85, 255};
static const ColorRGBA COLOR_BOARD_BG      = {24, 60, 190, 255};
static const ColorRGBA COLOR_BANNER_SEP    = {55, 95, 225, 255};

/* Empty slot */
static const ColorRGBA COLOR_SLOT_EMPTY    = {238, 242, 248, 255};
static const ColorRGBA COLOR_SLOT_RIM      = {15, 30, 85, 255};
static const ColorRGBA COLOR_SLOT_SHADOW   = {195, 202, 215, 255};

/* Player 1 (Red) */
static const ColorRGBA COLOR_P1_FILL       = {225, 45, 45, 255};
static const ColorRGBA COLOR_P1_BORDER     = {140, 20, 20, 255};
static const ColorRGBA COLOR_P1_HIGHLIGHT  = {245, 110, 110, 200};
static const ColorRGBA COLOR_P1_SPECULAR   = {255, 215, 215, 180};

/* Player 2 (Yellow) */
static const ColorRGBA COLOR_P2_FILL       = {245, 205, 30, 255};
static const ColorRGBA COLOR_P2_BORDER     = {160, 130, 15, 255};
static const ColorRGBA COLOR_P2_HIGHLIGHT  = {255, 235, 120, 200};
static const ColorRGBA COLOR_P2_SPECULAR   = {255, 255, 220, 190};

/* Game Over Modal */
static const ColorRGBA COLOR_MODAL_SHADOW  = {5, 10, 25, 130};
static const ColorRGBA COLOR_MODAL_BG      = {16, 24, 52, 235};
static const ColorRGBA COLOR_MODAL_BORDER  = {220, 235, 255, 220};
static const ColorRGBA COLOR_MODAL_SUB1    = {200, 215, 240, 220};
static const ColorRGBA COLOR_MODAL_SUB2    = {170, 190, 220, 200};

static SDL_Window   *g_window     = NULL;
static SDL_Renderer *g_renderer   = NULL;
static char         *g_prev_board = NULL;
static int           g_rows       = 0;
static int           g_cols       = 0;

int gui_init(int rows, int cols) {
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

    g_rows = rows;
    g_cols = cols;
    g_prev_board = malloc(rows * cols * sizeof(char));
    if (g_prev_board) {
        memset(g_prev_board, '.', rows * cols);
    }

    return 1;
}

void gui_shutdown(void) {
    if (g_prev_board) {
        free(g_prev_board);
        g_prev_board = NULL;
    }
    g_rows = 0;
    g_cols = 0;

    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }
    SDL_Quit();
}

static void draw_piece(int cx, int cy, char token) {
    if (token == 'X') {
        filledCircleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS,
                         COLOR_P1_FILL.r, COLOR_P1_FILL.g, COLOR_P1_FILL.b, COLOR_P1_FILL.a);
        circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS,
                   COLOR_P1_BORDER.r, COLOR_P1_BORDER.g, COLOR_P1_BORDER.b, COLOR_P1_BORDER.a);
        circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS - 7,
                   COLOR_P1_HIGHLIGHT.r, COLOR_P1_HIGHLIGHT.g, COLOR_P1_HIGHLIGHT.b, COLOR_P1_HIGHLIGHT.a);
        filledCircleRGBA(g_renderer, (Sint16)(cx - PIECE_RADIUS / 3), (Sint16)(cy - PIECE_RADIUS / 3), 5,
                         COLOR_P1_SPECULAR.r, COLOR_P1_SPECULAR.g, COLOR_P1_SPECULAR.b, COLOR_P1_SPECULAR.a);
    } else if (token == 'O') {
        filledCircleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS,
                         COLOR_P2_FILL.r, COLOR_P2_FILL.g, COLOR_P2_FILL.b, COLOR_P2_FILL.a);
        circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS,
                   COLOR_P2_BORDER.r, COLOR_P2_BORDER.g, COLOR_P2_BORDER.b, COLOR_P2_BORDER.a);
        circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS - 7,
                   COLOR_P2_HIGHLIGHT.r, COLOR_P2_HIGHLIGHT.g, COLOR_P2_HIGHLIGHT.b, COLOR_P2_HIGHLIGHT.a);
        filledCircleRGBA(g_renderer, (Sint16)(cx - PIECE_RADIUS / 3), (Sint16)(cy - PIECE_RADIUS / 3), 5,
                         COLOR_P2_SPECULAR.r, COLOR_P2_SPECULAR.g, COLOR_P2_SPECULAR.b, COLOR_P2_SPECULAR.a);
    } else {
        filledCircleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS,
                         COLOR_SLOT_EMPTY.r, COLOR_SLOT_EMPTY.g, COLOR_SLOT_EMPTY.b, COLOR_SLOT_EMPTY.a);
        circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS,
                   COLOR_SLOT_RIM.r, COLOR_SLOT_RIM.g, COLOR_SLOT_RIM.b, COLOR_SLOT_RIM.a);
        arcRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS - 1, 180, 360,
                COLOR_SLOT_SHADOW.r, COLOR_SLOT_SHADOW.g, COLOR_SLOT_SHADOW.b, COLOR_SLOT_SHADOW.a);
    }
}

static void render_board_view(const GameState *st, int hover_col, char hover_token, int hide_r, int hide_c) {
    int total_width  = st->cols * GUI_CELL_SIZE;
    int total_height = (st->rows + TOP_MARGIN_ROWS) * GUI_CELL_SIZE;

    /* Top banner area */
    boxRGBA(g_renderer, 0, 0, (Sint16)total_width, GUI_CELL_SIZE,
            COLOR_BANNER_BG.r, COLOR_BANNER_BG.g, COLOR_BANNER_BG.b, COLOR_BANNER_BG.a);

    /* Blue board area */
    boxRGBA(g_renderer, 0, GUI_CELL_SIZE, (Sint16)total_width, (Sint16)total_height,
            COLOR_BOARD_BG.r, COLOR_BOARD_BG.g, COLOR_BOARD_BG.b, COLOR_BOARD_BG.a);

    /* Header separator line */
    lineRGBA(g_renderer, 0, GUI_CELL_SIZE - 1, (Sint16)total_width, GUI_CELL_SIZE - 1,
             COLOR_BANNER_SEP.r, COLOR_BANNER_SEP.g, COLOR_BANNER_SEP.b, COLOR_BANNER_SEP.a);

    /* Render grid slots and placed pieces */
    for (int r = 0; r < st->rows; r++) {
        for (int c = 0; c < st->cols; c++) {
            int cx = c * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;
            int cy = (r + TOP_MARGIN_ROWS) * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;

            if (r == hide_r && c == hide_c) {
                draw_piece(cx, cy, '.');
            } else {
                draw_piece(cx, cy, st->board[r * st->cols + c]);
            }
        }
    }

    /* Hover preview piece (Ghost Piece) */
    if (hover_col >= 0 && hover_col < st->cols && is_valid_move(st, hover_col)) {
        int cx = hover_col * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;
        int cy = GUI_CELL_SIZE / 2;

        if (hover_token == 'X') {
            filledCircleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS,
                             COLOR_P1_FILL.r, COLOR_P1_FILL.g, COLOR_P1_FILL.b, 160);
            circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS, 255, 255, 255, 210);
            circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS - 6,
                       COLOR_P1_HIGHLIGHT.r, COLOR_P1_HIGHLIGHT.g, COLOR_P1_HIGHLIGHT.b, 140);
        } else if (hover_token == 'O') {
            filledCircleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS,
                             COLOR_P2_FILL.r, COLOR_P2_FILL.g, COLOR_P2_FILL.b, 160);
            circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS, 255, 255, 255, 210);
            circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS - 6,
                       COLOR_P2_HIGHLIGHT.r, COLOR_P2_HIGHLIGHT.g, COLOR_P2_HIGHLIGHT.b, 140);
        }

        /* Downward guide arrow pointing into column */
        filledTrigonRGBA(g_renderer,
                         (Sint16)(cx - 8), (Sint16)(cy + PIECE_RADIUS + 2),
                         (Sint16)(cx + 8), (Sint16)(cy + PIECE_RADIUS + 2),
                         (Sint16)cx,       (Sint16)(cy + PIECE_RADIUS + 8),
                         255, 255, 255, 220);
    }

    /* Top banner turn prompt */
    if (hover_token != 0) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "Player %c's turn", hover_token);
        int text_x = (total_width - (int)strlen(prompt) * 8) / 2;
        if (text_x < 10) text_x = 10;
        stringRGBA(g_renderer, (Sint16)text_x, 10, prompt, 210, 230, 255, 220);

        /* Undo shortcut indicator */
        stringRGBA(g_renderer, (Sint16)(total_width - 76), 10, "[u] undo", 180, 210, 255, 190);
    }
}

static void animate_piece_drop(const GameState *st, int col, int target_row, char token) {
    int start_y = GUI_CELL_SIZE / 2;
    int end_y   = (target_row + TOP_MARGIN_ROWS) * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;
    int cx      = col * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;

    int frames = 8 + target_row * 2;
    for (int f = 1; f <= frames; f++) {
        float t = (float)f / (float)frames;
        int cur_y = start_y + (int)((end_y - start_y) * (t * t));

        render_board_view(st, -1, 0, target_row, col);
        draw_piece(cx, cur_y, token);
        SDL_RenderPresent(g_renderer);
        SDL_Delay(12);
    }

    /* Subtle landing bounce */
    if (target_row > 0) {
        render_board_view(st, -1, 0, target_row, col);
        draw_piece(cx, end_y - 4, token);
        SDL_RenderPresent(g_renderer);
        SDL_Delay(14);

        render_board_view(st, -1, 0, target_row, col);
        draw_piece(cx, end_y, token);
        SDL_RenderPresent(g_renderer);
        SDL_Delay(14);
    }
}

void gui_render_board(const GameState *st) {
    if (!g_renderer || !g_window) return;

    /* Detect single placed piece to trigger smooth drop animation */
    if (g_prev_board && g_rows == st->rows && g_cols == st->cols) {
        int diff_count = 0;
        int diff_r = -1, diff_c = -1;

        for (int r = 0; r < st->rows; r++) {
            for (int c = 0; c < st->cols; c++) {
                char old_c = g_prev_board[r * st->cols + c];
                char new_c = st->board[r * st->cols + c];
                if (old_c != new_c) {
                    diff_count++;
                    if (old_c == '.' && (new_c == 'X' || new_c == 'O')) {
                        diff_r = r;
                        diff_c = c;
                    }
                }
            }
        }

        if (diff_count == 1 && diff_r != -1) {
            animate_piece_drop(st, diff_c, diff_r, st->board[diff_r * st->cols + diff_c]);
        }

        memcpy(g_prev_board, st->board, st->rows * st->cols);
    }

    render_board_view(st, -1, 0, -1, -1);
    SDL_RenderPresent(g_renderer);
}

int human_move_gui(const GameState *st, void *ctx) {
    char token = 'X';
    if (ctx) {
        token = *(char *)ctx;
    } else {
        int piece_count = 0;
        for (int i = 0; i < st->rows * st->cols; i++) {
            if (st->board[i] != '.') piece_count++;
        }
        token = (piece_count % 2 == 0) ? 'X' : 'O';
    }

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    int hover_col = mx / GUI_CELL_SIZE;
    if (hover_col < 0 || hover_col >= st->cols) {
        hover_col = -1;
    }

    render_board_view(st, hover_col, token, -1, -1);
    SDL_RenderPresent(g_renderer);

    SDL_Event event;
    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return MOVE_QUIT;
            }
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode k = event.key.keysym.sym;
                if (k == SDLK_u || (k == SDLK_z && (event.key.keysym.mod & KMOD_CTRL)) || k == SDLK_BACKSPACE) {
                    return MOVE_UNDO;
                }
                if (k == SDLK_F12) {
                    gui_save_screenshot("screenshot.bmp");
                    printf("Screenshot saved to screenshot.bmp\n");
                }
            }
            if (event.type == SDL_MOUSEMOTION) {
                int new_col = event.motion.x / GUI_CELL_SIZE;
                if (new_col < 0 || new_col >= st->cols) {
                    new_col = -1;
                }
                if (new_col != hover_col) {
                    hover_col = new_col;
                    render_board_view(st, hover_col, token, -1, -1);
                    SDL_RenderPresent(g_renderer);
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int col = event.button.x / GUI_CELL_SIZE;
                if (is_valid_move(st, col)) {
                    return col;
                }
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_EXPOSED) {
                render_board_view(st, hover_col, token, -1, -1);
                SDL_RenderPresent(g_renderer);
            }
        }
        SDL_Delay(16);
    }
}

static void render_win_overlay(int win_coords[4][2]) {
    int x0 = win_coords[0][1] * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;
    int y0 = (win_coords[0][0] + TOP_MARGIN_ROWS) * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;
    int x3 = win_coords[3][1] * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;
    int y3 = (win_coords[3][0] + TOP_MARGIN_ROWS) * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;

    /* Outer glow line */
    thickLineRGBA(g_renderer, (Sint16)x0, (Sint16)y0, (Sint16)x3, (Sint16)y3, 14, 255, 255, 255, 120);
    /* Inner crisp core beam */
    thickLineRGBA(g_renderer, (Sint16)x0, (Sint16)y0, (Sint16)x3, (Sint16)y3, 6, 255, 255, 255, 255);

    /* Highlight rings on each of the 4 winning discs */
    for (int i = 0; i < 4; i++) {
        int cx = win_coords[i][1] * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;
        int cy = (win_coords[i][0] + TOP_MARGIN_ROWS) * GUI_CELL_SIZE + GUI_CELL_SIZE / 2;

        circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS + 1, 255, 255, 255, 255);
        circleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, PIECE_RADIUS - 4, 255, 255, 255, 220);
        filledCircleRGBA(g_renderer, (Sint16)cx, (Sint16)cy, 7, 255, 255, 255, 255);
    }
}

static void render_game_over_modal(int winner, int total_width, int total_height) {
    int banner_w = 340;
    if (banner_w > total_width - 40) {
        banner_w = total_width - 40;
    }
    int banner_h = 100;
    int bx1 = (total_width - banner_w) / 2;
    int by1 = (total_height - banner_h) / 2;
    int bx2 = bx1 + banner_w;
    int by2 = by1 + banner_h;

    /* Modal card shadow */
    roundedBoxRGBA(g_renderer, (Sint16)(bx1 + 4), (Sint16)(by1 + 4), (Sint16)(bx2 + 4), (Sint16)(by2 + 4),
                   12, COLOR_MODAL_SHADOW.r, COLOR_MODAL_SHADOW.g, COLOR_MODAL_SHADOW.b, COLOR_MODAL_SHADOW.a);

    /* Modal card body */
    roundedBoxRGBA(g_renderer, (Sint16)bx1, (Sint16)by1, (Sint16)bx2, (Sint16)by2,
                   12, COLOR_MODAL_BG.r, COLOR_MODAL_BG.g, COLOR_MODAL_BG.b, COLOR_MODAL_BG.a);
    roundedRectangleRGBA(g_renderer, (Sint16)bx1, (Sint16)by1, (Sint16)bx2, (Sint16)by2,
                         12, COLOR_MODAL_BORDER.r, COLOR_MODAL_BORDER.g, COLOR_MODAL_BORDER.b, COLOR_MODAL_BORDER.a);

    /* Title text */
    const char *title;
    Uint8 tr, tg, tb;
    if (winner == 'X') {
        title = "PLAYER X WINS!";
        tr = COLOR_P1_FILL.r; tg = COLOR_P1_FILL.g; tb = COLOR_P1_FILL.b;
    } else if (winner == 'O') {
        title = "PLAYER O WINS!";
        tr = COLOR_P2_FILL.r; tg = COLOR_P2_FILL.g; tb = COLOR_P2_FILL.b;
    } else {
        title = "MATCH DRAW!";
        tr = 175; tg = 220; tb = 255;
    }

    int tx = bx1 + (banner_w - (int)strlen(title) * 8) / 2;
    int ty = by1 + 25;
    stringRGBA(g_renderer, (Sint16)tx, (Sint16)ty, title, tr, tg, tb, 255);
    stringRGBA(g_renderer, (Sint16)(tx + 1), (Sint16)ty, title, tr, tg, tb, 255);

    /* Subtitle instructions */
    const char *sub1 = "Game Finished";
    const char *sub2 = "Click or press Space / Esc to continue";
    int s1x = bx1 + (banner_w - (int)strlen(sub1) * 8) / 2;
    int s2x = bx1 + (banner_w - (int)strlen(sub2) * 8) / 2;

    stringRGBA(g_renderer, (Sint16)s1x, (Sint16)(by1 + 50), sub1,
               COLOR_MODAL_SUB1.r, COLOR_MODAL_SUB1.g, COLOR_MODAL_SUB1.b, COLOR_MODAL_SUB1.a);
    stringRGBA(g_renderer, (Sint16)s2x, (Sint16)(by1 + 68), sub2,
               COLOR_MODAL_SUB2.r, COLOR_MODAL_SUB2.g, COLOR_MODAL_SUB2.b, COLOR_MODAL_SUB2.a);
}

void gui_show_game_over(const GameState *st, int winner) {
    if (!g_renderer || !g_window) return;

    int win_coords[4][2];
    int has_win = 0;
    if (winner == 'X' || winner == 'O') {
        has_win = get_winning_line(st, (char)winner, win_coords);
    }

    int width  = st->cols * GUI_CELL_SIZE;
    int height = (st->rows + TOP_MARGIN_ROWS) * GUI_CELL_SIZE;

    int running = 1;
    while (running) {
        render_board_view(st, -1, 0, -1, -1);

        if (has_win) {
            render_win_overlay(win_coords);
        }

        render_game_over_modal(winner, width, height);

        SDL_RenderPresent(g_renderer);

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = 0;
                break;
            }
            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                running = 0;
                break;
            }
            if (ev.type == SDL_KEYDOWN) {
                SDL_Keycode k = ev.key.keysym.sym;
                if (k == SDLK_SPACE || k == SDLK_RETURN || k == SDLK_ESCAPE) {
                    running = 0;
                    break;
                }
            }
        }
        SDL_Delay(20);
    }
}

void gui_indicate_ai_thinking(const GameState *st) {
    if (!g_renderer || !g_window) return;

    render_board_view(st, -1, 0, -1, -1);

    int total_width = st->cols * GUI_CELL_SIZE;
    const char *msg = "AI thinking...";
    int text_x = (total_width - (int)strlen(msg) * 8) / 2;
    if (text_x < 10) text_x = 10;

    stringRGBA(g_renderer, (Sint16)text_x, 10, msg, COLOR_P2_FILL.r, COLOR_P2_FILL.g, COLOR_P2_FILL.b, 240);
    stringRGBA(g_renderer, (Sint16)(text_x + 1), 10, msg, COLOR_P2_FILL.r, COLOR_P2_FILL.g, COLOR_P2_FILL.b, 240);

    SDL_RenderPresent(g_renderer);
    SDL_PumpEvents();
    SDL_Delay(350);
}

int gui_save_screenshot(const char *filename) {
    if (!g_renderer || !g_window) return 0;

    int width = g_cols * GUI_CELL_SIZE;
    int height = (g_rows + TOP_MARGIN_ROWS) * GUI_CELL_SIZE;

    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!surface) return 0;

    if (SDL_RenderReadPixels(g_renderer, NULL, SDL_PIXELFORMAT_ARGB8888, surface->pixels, surface->pitch) != 0) {
        SDL_FreeSurface(surface);
        return 0;
    }

    int res = SDL_SaveBMP(surface, filename);
    SDL_FreeSurface(surface);
    return (res == 0);
}

void gui_render_state(const GameState *st, int hover_col, char hover_token) {
    if (!g_renderer || !g_window) return;
    render_board_view(st, hover_col, hover_token, -1, -1);
    SDL_RenderPresent(g_renderer);
}

int gui_render_and_save_screenshot(const GameState *st, int hover_col, char hover_token, int winner_token, const char *filename) {
    if (!g_renderer || !g_window) return 0;

    render_board_view(st, hover_col, hover_token, -1, -1);

    if (winner_token == 'X' || winner_token == 'O') {
        int win_coords[4][2];
        if (get_winning_line(st, (char)winner_token, win_coords)) {
            render_win_overlay(win_coords);
        }

        int width  = st->cols * GUI_CELL_SIZE;
        int height = (st->rows + TOP_MARGIN_ROWS) * GUI_CELL_SIZE;
        render_game_over_modal(winner_token, width, height);
    }

    int res = gui_save_screenshot(filename);
    SDL_RenderPresent(g_renderer);
    return res;
}
