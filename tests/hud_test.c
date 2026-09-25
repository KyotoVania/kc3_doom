/* kc3
 * Copyright from 2022 to 2026 kmx.io <contact@kmx.io>
 *
 * Permission is hereby granted to use this software granted the above
 * copyright notice and this permission paragraph are included in all
 * copies and substantial portions of this software.
 *
 * THIS SOFTWARE IS PROVIDED "AS-IS" WITHOUT ANY GUARANTEE OF
 * PURPOSE AND PERFORMANCE. IN NO EVENT WHATSOEVER SHALL THE
 * AUTHOR BE CONSIDERED LIABLE FOR THE USE AND PERFORMANCE OF
 * THIS SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include <cairo.h>
#include "hud.h"

#define HUD_TEST_GRID_W 24
#define HUD_TEST_GRID_H 19

const char *g_hud_test_engine_grid_get_stub =
  "engine_grid_get: hud_test stub, not the engine implementation";

static s_engine g_hud_test_engine;

static const char *g_hud_test_grid[HUD_TEST_GRID_H] = {
  "111111111111111111111111",
  "1......1...............1",
  "1......1...2222........1",
  "1......D...2..2........1",
  "1......1...2..2....2...1",
  "1..22..1...2222....2...1",
  "1..22..1...........2...1",
  "1......111D111.....2...1",
  "1......................1",
  "1......2..........2....1",
  "1..2...2...222....2....1",
  "1..2...2...2......2....1",
  "1..2...D...2..2...2....1",
  "1..2...2...2..2........1",
  "1......2...2222....2...1",
  "1......2...........2...1",
  "1..1...22222222222.2...1",
  "1..1...............2...1",
  "1111111111111111111X1111"
};

char engine_grid_get (const s_engine *engine, int x, int y)
{
  if (x < 0 || y < 0 || x >= engine->grid_w || y >= engine->grid_h)
    return '1';
  return engine->grid[y][x];
}

static void hud_test_base (s_hud *hud);
static bool hud_test_grid_init (void);
static bool hud_test_write (const s_hud *hud, const char *path);

int main (void)
{
  s_hud hud;
  int fails;
  fails = 0;
  if (! hud_test_grid_init())
    return 1;
  memset(&hud, 0, sizeof(hud));
  hud.state = HUD_TITLE;
  hud.time = 0.6;
  if (! hud_test_write(&hud, "tests/out/hud_title.png"))
    fails++;
  hud_test_base(&hud);
  hud.weapon = 0;
  hud.weapon_name = "printf";
  hud.fire_t = 0.08;
  hud.bob = 2.0;
  hud.moving = 1.0;
  hud.msg[0] = "SIGSEGV corrigé. NULL-check ajouté.";
  hud.msg_t[0] = 3.5;
  hud.msg[1] = "+20 breakpoints.";
  hud.msg_t[1] = 1.5;
  hud.msg_count = 2;
  hud.thomas_line = "Règle d'or : un test RED, puis GREEN.";
  hud.has_thomas = true;
  hud.thomas_x = 18.5;
  hud.thomas_y = 16.5;
  hud.show_map = true;
  hud.mob_xy[0] = 3.5;
  hud.mob_xy[1] = 8.5;
  hud.mob_xy[2] = 12.5;
  hud.mob_xy[3] = 1.5;
  hud.mob_xy[4] = 15.5;
  hud.mob_xy[5] = 10.5;
  hud.mob_xy[6] = 20.5;
  hud.mob_xy[7] = 13.5;
  hud.mob_xy[8] = 7.5;
  hud.mob_xy[9] = 17.5;
  hud.mob_count = 5;
  if (! hud_test_write(&hud, "tests/out/hud_play_printf.png"))
    fails++;
  hud_test_base(&hud);
  hud.weapon = 1;
  hud.weapon_name = "gdb";
  hud.hurt_t = 0.2;
  hud.hp = 40;
  hud.grin_t = 0;
  if (! hud_test_write(&hud, "tests/out/hud_play_gdb_hurt.png"))
    fails++;
  hud_test_base(&hud);
  hud.weapon = 2;
  hud.weapon_name = "ASan";
  hud.owned[2] = true;
  hud.fire_t = 0.06;
  hud.intro = 2;
  if (! hud_test_write(&hud, "tests/out/hud_play_asan_intro.png"))
    fails++;
  hud_test_base(&hud);
  hud.state = HUD_DEAD;
  hud.state_t = 1;
  hud.hp = 0;
  if (! hud_test_write(&hud, "tests/out/hud_dead.png"))
    fails++;
  hud_test_base(&hud);
  hud.state = HUD_INTER;
  hud.state_t = 1;
  hud.kills = 10;
  hud.total = 12;
  hud.facts = 3;
  hud.ftotal = 4;
  hud.level_time = 83;
  hud.outro = "« Bien. Maintenant, le serveur HTTP. »";
  if (! hud_test_write(&hud, "tests/out/hud_inter.png"))
    fails++;
  hud_test_base(&hud);
  hud.state = HUD_WIN;
  hud.state_t = 2;
  hud.g_kills = 30;
  hud.g_total = 34;
  hud.g_facts = 9;
  hud.g_ftotal = 12;
  hud.g_time = 754;
  if (! hud_test_write(&hud, "tests/out/hud_win.png"))
    fails++;
  if (fails) {
    fprintf(stderr, "hud_test: %d failure(s)\n", fails);
    return 1;
  }
  puts("hud_test: OK");
  return 0;
}

static void hud_test_base (s_hud *hud)
{
  memset(hud, 0, sizeof(*hud));
  hud->state = HUD_PLAY;
  hud->time = 0.6;
  hud->px = 5.5;
  hud->py = 9.5;
  hud->pa = 0.6;
  hud->hp = 100;
  hud->armor = 50;
  hud->ammo = 120;
  hud->weapon = 0;
  hud->owned[0] = true;
  hud->owned[1] = true;
  hud->owned[2] = false;
  hud->weapon_name = "printf";
  hud->kills = 7;
  hud->total = 12;
  hud->facts = 2;
  hud->ftotal = 4;
  hud->level_time = 83;
  hud->level_name = "E1M1 : libkc3/";
  hud->level_sub = "Le runtime";
}

static bool hud_test_grid_init (void)
{
  int y;
  for (y = 0; y < HUD_TEST_GRID_H; y++) {
    if (strlen(g_hud_test_grid[y]) != HUD_TEST_GRID_W) {
      fprintf(stderr, "hud_test: bad grid row %d\n", y);
      return false;
    }
    memcpy(g_hud_test_engine.grid[y], g_hud_test_grid[y],
           HUD_TEST_GRID_W);
  }
  g_hud_test_engine.grid_w = HUD_TEST_GRID_W;
  g_hud_test_engine.grid_h = HUD_TEST_GRID_H;
  return true;
}

static bool hud_test_write (const s_hud *hud, const char *path)
{
  cairo_t *cr;
  cairo_status_t status;
  cairo_surface_t *surface;
  surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, ENGINE_W,
                                       ENGINE_H);
  cr = cairo_create(surface);
  cairo_set_source_rgb(cr, .5, .5, .5);
  cairo_paint(cr);
  hud_draw(cr, hud, &g_hud_test_engine);
  cairo_destroy(cr);
  status = cairo_surface_write_to_png(surface, path);
  cairo_surface_destroy(surface);
  if (status != CAIRO_STATUS_SUCCESS) {
    fprintf(stderr, "hud_test: %s: %s\n", path,
            cairo_status_to_string(status));
    return false;
  }
  printf("hud_test: wrote %s\n", path);
  return true;
}
