#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#define SCREEN_W 960
#define SCREEN_H 540
#define MAX_BONES 6

typedef enum { STATE_PLAY = 0, STATE_OVER = 1 } GameState;

typedef struct {
    int jump;     /* 1 = нажата кнопка прыжка (X) */
    int restart;  /* 1 = нажата кнопка рестарта после проигрыша */
} Input;

typedef struct {
    float x;
    int active;
    int passed;
} Bone;

typedef struct {
    float tito_y;
    float vy;
    float speed;
    int on_ground;
    Bone bones[MAX_BONES];
    int score;
    int lives;
    int spawn_timer;
    GameState state;
    uint32_t seed;
} Game;

void game_init(Game *g, uint32_t seed);
void game_update(Game *g, Input in);
void game_draw(const Game *g, uint32_t *fb); /* fb: SCREEN_W * SCREEN_H пикселей, ARGB */

#endif
