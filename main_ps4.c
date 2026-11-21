/*
 * Платформенный слой PS4. Игровая логика (game.c) одинаковая на ПК и на PS4.
 *
 * ВАЖНО: здесь нет рабочего вывода на экран и чтения геймпада. Эти вызовы
 * берутся из официальных примеров OpenOrbis (samples) для твоей версии
 * toolchain и вставляются в места, помеченные TODO. Без них программа
 * ничего не покажет на экране.
 */
#include "game.h"
#include <stdint.h>

static uint32_t framebuffer[SCREEN_W * SCREEN_H];

/* TODO: из samples OpenOrbis: инициализация VideoOut и выделение буферов */
static void platform_init(void) {}

/* TODO: из samples OpenOrbis: чтение состояния пэда (крестик = прыжок) */
static Input platform_read_input(void)
{
    Input in = {0, 0};
    return in;
}

/* TODO: из samples OpenOrbis: вывод framebuffer на экран (учти порядок байт ARGB/BGRA) */
static void platform_present(const uint32_t *fb)
{
    (void)fb;
}

int main(void)
{
    Game g;
    platform_init();
    game_init(&g, 42u);

    for (;;) {
        Input in = platform_read_input();
        game_update(&g, in);
        game_draw(&g, framebuffer);
        platform_present(framebuffer);
    }
    return 0;
}
