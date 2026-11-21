#include "game.h"
#include <string.h>

#define TITO_X       120.0f
#define TITO_W       90
#define TITO_H       110
#define BONE_W       44
#define BONE_H       50
#define GROUND_Y     (SCREEN_H - 80)
#define GRAVITY      0.9f
#define JUMP_V       (-16.0f)
#define START_SPEED  6.0f
#define MAX_SPEED    14.0f
#define START_LIVES  3

static uint32_t next_rand(uint32_t *s)
{
    *s = *s * 1664525u + 1013904223u;
    return *s >> 8;
}

void game_init(Game *g, uint32_t seed)
{
    memset(g, 0, sizeof *g);
    g->seed = seed ? seed : 1u;
    g->tito_y = (float)(GROUND_Y - TITO_H);
    g->on_ground = 1;
    g->speed = START_SPEED;
    g->lives = START_LIVES;
    g->spawn_timer = 60;
    g->state = STATE_PLAY;
}

static int rects_overlap(float ax, float ay, float aw, float ah,
                         float bx, float by, float bw, float bh)
{
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

void game_update(Game *g, Input in)
{
    if (g->state == STATE_OVER) {
        if (in.restart) {
            game_init(g, g->seed + 1u);
        }
        return;
    }

    /* Прыжок и гравитация */
    if (in.jump && g->on_ground) {
        g->vy = JUMP_V;
        g->on_ground = 0;
    }
    g->vy += GRAVITY;
    g->tito_y += g->vy;
    if (g->tito_y >= (float)(GROUND_Y - TITO_H)) {
        g->tito_y = (float)(GROUND_Y - TITO_H);
        g->vy = 0.0f;
        g->on_ground = 1;
    }

    /* Спавн костей */
    if (--g->spawn_timer <= 0) {
        for (int i = 0; i < MAX_BONES; i++) {
            if (!g->bones[i].active) {
                g->bones[i].active = 1;
                g->bones[i].passed = 0;
                g->bones[i].x = (float)SCREEN_W;
                break;
            }
        }
        g->spawn_timer = 50 + (int)(next_rand(&g->seed) % 50u);
    }

    /* Движение, очки и столкновения */
    for (int i = 0; i < MAX_BONES; i++) {
        Bone *b = &g->bones[i];
        if (!b->active) {
            continue;
        }
        b->x -= g->speed;
        if (b->x + BONE_W < 0.0f) {
            b->active = 0;
            continue;
        }
        if (!b->passed && b->x + BONE_W < TITO_X) {
            b->passed = 1;
            g->score++;
        }
        if (rects_overlap(TITO_X, g->tito_y, TITO_W, TITO_H,
                          b->x, (float)(GROUND_Y - BONE_H), BONE_W, BONE_H)) {
            b->active = 0;
            g->lives--;
            if (g->lives <= 0) {
                g->lives = 0;
                g->state = STATE_OVER;
                return;
            }
        }
    }

    if (g->speed < MAX_SPEED) {
        g->speed += 0.002f;
    }
}

static void fill_rect(uint32_t *fb, int x, int y, int w, int h, uint32_t color)
{
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= SCREEN_H) {
            continue;
        }
        for (int i = x; i < x + w; i++) {
            if (i < 0 || i >= SCREEN_W) {
                continue;
            }
            fb[j * SCREEN_W + i] = color;
        }
    }
}

void game_draw(const Game *g, uint32_t *fb)
{
    const int tx = (int)TITO_X;
    const int ty = (int)g->tito_y;

    fill_rect(fb, 0, 0, SCREEN_W, SCREEN_H, 0xFF87CEEBu);                     /* небо */
    fill_rect(fb, 0, GROUND_Y, SCREEN_W, SCREEN_H - GROUND_Y, 0xFF8B5A2Bu);   /* земля */

    for (int i = 0; i < MAX_BONES; i++) {
        if (g->bones[i].active) {
            fill_rect(fb, (int)g->bones[i].x, GROUND_Y - BONE_H, BONE_W, BONE_H, 0xFFF5F5DCu);
        }
    }

    /* Тито: чёрное тело, уши, глаз */
    fill_rect(fb, tx, ty, TITO_W, TITO_H, 0xFF111111u);
    fill_rect(fb, tx + 10, ty - 14, 22, 16, 0xFF111111u);
    fill_rect(fb, tx + TITO_W - 32, ty - 14, 22, 16, 0xFF111111u);
    fill_rect(fb, tx + 22, ty + 20, 8, 8, 0xFFFFFFFFu);

    /* HUD: жизни (красные квадраты) и очки (полоска) */
    for (int i = 0; i < g->lives; i++) {
        fill_rect(fb, 20 + i * 34, 20, 26, 26, 0xFFE63946u);
    }
    int bar = g->score * 6;
    if (bar > 400) {
        bar = 400;
    }
    fill_rect(fb, 20, 56, bar, 12, 0xFFFFD166u);

    if (g->state == STATE_OVER) {
        fill_rect(fb, 280, 200, 400, 120, 0xCC000000u);
    }
}
