#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#define SCREEN_W 960
#define SCREEN_H 540
#define MAX_BONES 6

/* события кадра: для звука, вибрации и подсветки */
#define EV_JUMP   (1u << 0)
#define EV_SCORE  (1u << 1)
#define EV_HIT    (1u << 2)
#define EV_OVER   (1u << 3)

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
    uint32_t events; /* EV_* за последний кадр */
} Game;

void game_init(Game *g, uint32_t seed);
void game_update(Game *g, Input in);
void game_draw(const Game *g, uint32_t *fb); /* fb: SCREEN_W * SCREEN_H пикселей, ARGB */

#endif
