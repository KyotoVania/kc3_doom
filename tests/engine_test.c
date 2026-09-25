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
#include <math.h>
#include <stdio.h>
#include "engine.h"

static const char *g_map[] = {
  "111111111111111111111111",
  "1....1.......2.........1",
  "1....1.......2.........1",
  "1....D.......2222D222..1",
  "1....1.............2...1",
  "11D111....6666.....2...1",
  "1.........6..6.....2...1",
  "1.........6..6.....D...1",
  "1.........6..6.....2...1",
  "1.........66D6.....22221",
  "1......................1",
  "122D222222222..........1",
  "1......2....1..........1",
  "1......2....1..........1",
  "1......D....1111D1111111",
  "1......2....1..........1",
  "1......2....1..........1",
  "1......2....1..........1",
  "11111111111111111X111111"
};

#define MAP_H (sizeof(g_map) / sizeof(g_map[0]))

static int g_fail;

static void check (int ok, const char *name)
{
  if (! ok) {
    printf("engine_test: FAIL: %s\n", name);
    g_fail = 1;
  }
}

static void test_textures_init (s_engine *engine)
{
  int dx;
  int dy;
  uint32_t id;
  int r2;
  int x;
  int y;
  for (id = 0; id < TEX_COUNT; id++)
    for (y = 0; y < ENGINE_TEX; y++)
      for (x = 0; x < ENGINE_TEX; x++)
        engine->tex[id][y * ENGINE_TEX + x] =
          0xff000000 | (((id * 37) & 0xff) << 16) |
          ((x * 4) << 8) | (y * 4);
  for (id = TEX_SEGV_WALK_0; id < TEX_COUNT; id++)
    for (y = 0; y < ENGINE_TEX; y++)
      for (x = 0; x < ENGINE_TEX; x++) {
        dx = x - 32;
        dy = y - 32;
        r2 = dx * dx + dy * dy;
        if (r2 > 28 * 28)
          engine->tex[id][y * ENGINE_TEX + x] = 0;
      }
}

int main (void)
{
  cairo_t *cr;
  s_engine engine;
  cairo_surface_t *out;
  s_engine_hit hit;
  unsigned long sum;
  int i;
  const s_engine_sprite sprites[] = {
    {4.5, 2.5, TEX_SEGV_WALK_0, 0.8, 0.0, false},
    {4.5, 3.5, TEX_ITEM_COFFEE, 0.35, 0.25, true},
    {7.5, 1.5, TEX_PROJ, 0.3, 0.35, false}
  };
  if (! engine_init(&engine)) {
    printf("engine_test: FAIL: engine_init\n");
    return 1;
  }
  check(engine_grid_load(&engine, g_map, MAP_H), "grid_load");
  check(engine.grid_w == 24 && engine.grid_h == (int) MAP_H,
        "grid dims");
  check(engine_los(&engine, 1.5, 1.5, 4.5, 1.5), "los same room");
  check(! engine_los(&engine, 1.5, 1.5, 6.5, 1.5),
        "los through wall");
  check(engine_los(&engine, 3.0, 3.0, 3.0, 3.0),
        "los same point");
  check(engine_los(&engine, 1.5, 1.5, 1.5, 4.5), "los axial dx=0");
  check(! engine_los(&engine, 1.5, 1.5, 1.5, 6.5),
        "los axial through wall");
  hit = engine_cast(&engine, 1.5, 1.5, 1.0, 0.0);
  check(fabs(hit.d - 3.5) <= 1e-9, "cast distance 3.5");
  check(hit.mx == 5 && hit.my == 1 && hit.side == 0, "cast cell");
  test_textures_init(&engine);
  engine_sprites_clear(&engine);
  for (i = 0; i < 3; i++)
    check(engine_sprite_add(&engine, &sprites[i]), "sprite_add");
  engine_render(&engine, 1.5, 1.5, 0.3, TEX_FLOOR_TILE,
                TEX_FLOOR_PANEL, 14.0);
  sum = 0;
  for (i = 0; i < ENGINE_W * ENGINE_VH; i++)
    sum += engine.frame[i];
  check(sum > 0, "frame not black");
  for (i = 0; i < ENGINE_W; i++)
    if (engine.zbuf[i] <= 0.0) {
      check(0, "zbuf positive");
      break;
    }
  out = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, ENGINE_W,
                                   ENGINE_H);
  cr = cairo_create(out);
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_paint(cr);
  engine_blit(&engine, cr, ENGINE_W, ENGINE_H);
  cairo_destroy(cr);
  check(cairo_surface_write_to_png(out, "tests/out/engine_e1m1.png")
        == CAIRO_STATUS_SUCCESS, "write png");
  cairo_surface_destroy(out);
  engine_clean(&engine);
  if (g_fail)
    return 1;
  printf("engine_test: OK\n");
  return 0;
}
