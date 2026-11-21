#include "game.h"
#include <assert.h>
#include <stdio.h>

static uint32_t framebuffer[SCREEN_W * SCREEN_H];

static void test_init(void)
{
    Game g;
    game_init(&g, 7);
    assert(g.state == STATE_PLAY);
    assert(g.lives == 3);
    assert(g.score == 0);
    assert(g.on_ground == 1);
}

static void test_jump_and_land(void)
{
    Game g;
    game_init(&g, 7);
    float ground = g.tito_y;
    game_update(&g, (Input){1, 0});
    assert(g.tito_y < ground);
    assert(g.on_ground == 0);
    for (int i = 0; i < 200; i++) {
        game_update(&g, (Input){0, 0});
    }
    assert(g.on_ground == 1);
    assert(g.tito_y == ground);
}

static void test_collision_costs_life(void)
{
    Game g;
    game_init(&g, 7);
    g.bones[0].active = 1;
    g.bones[0].x = 120.0f;
    int before = g.lives;
    game_update(&g, (Input){0, 0});
    assert(g.lives == before - 1);
    assert(g.bones[0].active == 0);
}

static void test_game_over_and_restart(void)
{
    Game g;
    game_init(&g, 7);
    g.lives = 1;
    g.bones[0].active = 1;
    g.bones[0].x = 120.0f;
    game_update(&g, (Input){0, 0});
    assert(g.state == STATE_OVER);
    game_update(&g, (Input){0, 1});
    assert(g.state == STATE_PLAY);
    assert(g.lives == 3);
}

static void test_long_run_no_crash(void)
{
    Game g;
    game_init(&g, 12345);
    int games = 0;
    int max_score = 0;
    for (int frame = 0; frame < 200000; frame++) {
        if (g.state == STATE_OVER) {
            game_update(&g, (Input){0, 1});
            games++;
            continue;
        }
        Input in = {0, 0};
        for (int i = 0; i < MAX_BONES; i++) {
            const Bone *b = &g.bones[i];
            if (b->active && b->x > 0.0f && b->x < 250.0f) {
                in.jump = 1;
            }
        }
        game_update(&g, in);
        assert(g.tito_y <= 350.0f + 0.01f);
        assert(g.lives >= 0 && g.lives <= 3);
        if (g.score > max_score) {
            max_score = g.score;
        }
        if (frame % 1000 == 0) {
            game_draw(&g, framebuffer);
        }
    }
    printf("long run: games=%d best_score=%d\n", games, max_score);
}

int main(void)
{
    test_init();
    test_jump_and_land();
    test_collision_costs_life();
    test_game_over_and_restart();
    test_long_run_no_crash();
    printf("ALL TESTS PASSED\n");
    return 0;
}
