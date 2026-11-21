/*
 * Платформенный слой PS4: геймпад, вибрация, подсветка DualShock 4, звук.
 * Все функции взяты из headers OpenOrbis (include/orbis/Pad.h, AudioOut.h, UserService.h).
 *
 * ЕЩЁ НЕТ: вывода картинки (VideoOut). game_draw() заполняет framebuffer,
 * но на экран он пока не выводится.
 */
#include "game.h"
#include <stdint.h>
#include <string.h>

#include <orbis/libkernel.h>
#include <orbis/AudioOut.h>
#include <orbis/Pad.h>
#include <orbis/UserService.h>

static uint32_t framebuffer[SCREEN_W * SCREEN_H];

static int32_t pad_handle = -1;
static int32_t audio_handle = -1;

/* ---------- Звук: квадратные тона, 256 стерео-кадров за вызов ---------- */
#define AUDIO_RATE   48000
#define AUDIO_FRAMES 256

static int16_t audio_buf[AUDIO_FRAMES * 2];
static int tone_freq = 0;
static int tone_left = 0;
static int tone_phase = 0;

static void play_tone(int freq, int ms)
{
    tone_freq = freq;
    tone_left = AUDIO_RATE * ms / 1000;
    tone_phase = 0;
}

static void audio_pump(void)
{
    if (audio_handle < 0) {
        return;
    }
    for (int i = 0; i < AUDIO_FRAMES; i++) {
        int16_t s = 0;
        if (tone_left > 0 && tone_freq > 0) {
            int period = AUDIO_RATE / tone_freq;
            s = (tone_phase < period / 2) ? 6000 : -6000;
            tone_phase = (tone_phase + 1) % period;
            tone_left--;
        }
        audio_buf[2 * i] = s;
        audio_buf[2 * i + 1] = s;
    }
    sceAudioOutOutput(audio_handle, audio_buf); /* блокируется до проигрывания блока */
}

/* ---------- Вибрация ---------- */
static int vibe_frames = 0;

static void vibrate(int frames)
{
    vibe_frames = frames;
}

static void vibe_update(void)
{
    OrbisPadVibeParam p;
    memset(&p, 0, sizeof p);
    if (pad_handle >= 0 && vibe_frames > 0) {
        p.lgMotor = 255;
        p.smMotor = 255;
        vibe_frames--;
    }
    if (pad_handle >= 0) {
        scePadSetVibration(pad_handle, &p);
    }
}

/* ---------- Подсветка DualShock 4 ---------- */
static void lightbar_for(const Game *g)
{
    OrbisPadColor c;
    if (g->state == STATE_OVER) {
        c.r = 0;   c.g = 0;   c.b = 255;   /* синий: конец игры */
    } else if (g->lives >= 3) {
        c.r = 0;   c.g = 255; c.b = 0;     /* зелёный: все жизни */
    } else if (g->lives == 2) {
        c.r = 255; c.g = 200; c.b = 0;     /* жёлтый */
    } else {
        c.r = 255; c.g = 0;   c.b = 0;     /* красный: последняя жизнь */
    }
    if (pad_handle >= 0) {
        scePadSetLightBar(pad_handle, &c);
    }
}

/* ---------- Ввод ---------- */
static uint32_t read_buttons(void)
{
    OrbisPadData d;
    memset(&d, 0, sizeof d);
    if (pad_handle >= 0 && scePadReadState(pad_handle, &d) == 0) {
        return d.buttons;
    }
    return 0;
}

int main(void)
{
    Game g;
    int32_t user = ORBIS_USER_SERVICE_USER_ID_SYSTEM;

    sceUserServiceInitialize(NULL);
    sceUserServiceGetInitialUser(&user);

    if (scePadInit() == 0) {
        pad_handle = scePadOpen(user, 0, 0, NULL);
    }

    if (sceAudioOutInit() == 0) {
        audio_handle = sceAudioOutOpen(ORBIS_USER_SERVICE_USER_ID_SYSTEM,
                                       ORBIS_AUDIO_OUT_PORT_TYPE_MAIN, 0,
                                       AUDIO_FRAMES, AUDIO_RATE,
                                       ORBIS_AUDIO_OUT_PARAM_FORMAT_S16_STEREO);
    }

    game_init(&g, 42u);
    lightbar_for(&g);

    int last_lives = g.lives;
    GameState last_state = g.state;
    uint32_t prev = 0;
    for (;;) {
        uint32_t b = read_buttons();
        uint32_t pressed = b & ~prev; /* нажатия на этом кадре */
        prev = b;

        Input in;
        in.jump = (pressed & ORBIS_PAD_BUTTON_CROSS) != 0;
        in.restart = (pressed & ORBIS_PAD_BUTTON_CROSS) != 0;

        game_update(&g, in);

        if (g.events & EV_JUMP) {
            play_tone(880, 80);
        }
        if (g.events & EV_SCORE) {
            play_tone(1320, 60);
        }
        if (g.events & EV_HIT) {
            play_tone(110, 300);
            vibrate(20);
        }
        if (g.events & EV_OVER) {
            play_tone(55, 600);
            vibrate(40);
        }
        /* подсветка обновляется при смене жизней или состояния (в т.ч. после рестарта) */
        if (g.lives != last_lives || g.state != last_state) {
            lightbar_for(&g);
            last_lives = g.lives;
            last_state = g.state;
        }

        vibe_update();

        game_draw(&g, framebuffer); /* TODO: вывод framebuffer на экран (VideoOut) */

        /* 3 блока по 256 кадров ≈ 16 мс: это и есть частота кадров, звук блокирует цикл */
        audio_pump();
        audio_pump();
        audio_pump();
    }
    return 0;
}
