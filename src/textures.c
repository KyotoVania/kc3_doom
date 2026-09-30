/* kc3
 * Copyright from 2022 to 2026 kmx.io <contact@kmx.io>
 * Copyright 2026 KyotoVania
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
#include <stdlib.h>
#include <string.h>
#include "engine.h"

#define TEX_PI 3.14159265358979323846
#define TEX_2PI 6.28318530717958647692

static uint32_t g_seed;

static void draw_boss (cairo_t *cr, int p);
static void draw_dfree (cairo_t *cr, int p);
static void draw_floor_carpet (cairo_t *cr, int p);
static void draw_floor_dark (cairo_t *cr, int p);
static void draw_floor_grate (cairo_t *cr, int p);
static void draw_floor_panel (cairo_t *cr, int p);
static void draw_floor_tile (cairo_t *cr, int p);
static void draw_free1 (cairo_t *cr, int p);
static void draw_item_ammo (cairo_t *cr, int p);
static void draw_item_armor (cairo_t *cr, int p);
static void draw_item_asan (cairo_t *cr, int p);
static void draw_item_coffee (cairo_t *cr, int p);
static void draw_item_fact (cairo_t *cr, int p);
static void draw_item_gdb (cairo_t *cr, int p);
static void draw_leak (cairo_t *cr, int p);
static void draw_proj (cairo_t *cr, int p);
static void draw_bproj (cairo_t *cr, int p);
static void draw_race (cairo_t *cr, int p);
static void draw_segv (cairo_t *cr, int p);
static void draw_thomas (cairo_t *cr, int p);
static void draw_wall_1 (cairo_t *cr, int p);
static void draw_wall_2 (cairo_t *cr, int p);
static void draw_wall_3 (cairo_t *cr, int p);
static void draw_wall_4 (cairo_t *cr, int p);
static void draw_wall_5 (cairo_t *cr, int p);
static void draw_wall_6 (cairo_t *cr, int p);
static void draw_wall_d (cairo_t *cr, int p);
static void draw_wall_x (cairo_t *cr, int p);
static bool tex_color_parse (const char *str, double *r, double *g,
                             double *b, double *a);
static void tex_bricks (cairo_t *cr, const int rgb[3],
                        const char *mort);
static void tex_ellipse_path (cairo_t *cr, double x, double y,
                              double rx, double ry);
static void tex_frect (cairo_t *cr, const char *col, double x,
                       double y, double w, double h);
static void tex_ghost (cairo_t *cr, double x, const char *c, int p);
static int tex_hex (char c);
static void tex_oct (cairo_t *cr, double x, double y, double r,
                     const char *c);
static void tex_puddle (cairo_t *cr, const char *c, double w,
                        const char *label, const char *lc);
static void tex_quadratic_to (cairo_t *cr, double x1, double y1,
                              double x2, double y2);
static void tex_rack (cairo_t *cr, const char *label);
static bool tex_render (s_engine *engine, int id,
                        void (*draw) (cairo_t *cr, int p), int p);
static double tex_rnd (void);
static void tex_source (cairo_t *cr, const char *col);
static void tex_source_rgb (cairo_t *cr, int r, int g, int b);
static void tex_speckle (cairo_t *cr, const char *col, int n,
                         double a);
static void tex_srect (cairo_t *cr, const char *col, double x,
                       double y, double w, double h);
static void tex_txt (cairo_t *cr, const char *s, double x, double y,
                     const char *font, const char *col, char align);

double textures_rnd_test (uint32_t seed, int n)
{
  double r;
  int i;
  g_seed = seed;
  r = 0.0;
  for (i = 0; i < n; i++)
    r = tex_rnd();
  return r;
}

bool textures_init (s_engine *engine)
{
  g_seed = 7;
  return
    tex_render(engine, TEX_WALL_1, draw_wall_1, 0) &&
    tex_render(engine, TEX_WALL_2, draw_wall_2, 0) &&
    tex_render(engine, TEX_WALL_3, draw_wall_3, 0) &&
    tex_render(engine, TEX_WALL_4, draw_wall_4, 0) &&
    tex_render(engine, TEX_WALL_5, draw_wall_5, 0) &&
    tex_render(engine, TEX_WALL_6, draw_wall_6, 0) &&
    tex_render(engine, TEX_WALL_D, draw_wall_d, 0) &&
    tex_render(engine, TEX_WALL_X, draw_wall_x, 0) &&
    tex_render(engine, TEX_FLOOR_TILE, draw_floor_tile, 0) &&
    tex_render(engine, TEX_FLOOR_GRATE, draw_floor_grate, 0) &&
    tex_render(engine, TEX_FLOOR_CARPET, draw_floor_carpet, 0) &&
    tex_render(engine, TEX_FLOOR_PANEL, draw_floor_panel, 0) &&
    tex_render(engine, TEX_FLOOR_DARK, draw_floor_dark, 0) &&
    tex_render(engine, TEX_SEGV_WALK_0, draw_segv, 0) &&
    tex_render(engine, TEX_SEGV_WALK_1, draw_segv, 1) &&
    tex_render(engine, TEX_SEGV_ATK, draw_segv, 2) &&
    tex_render(engine, TEX_SEGV_DEAD, draw_segv, 3) &&
    tex_render(engine, TEX_LEAK_WALK_0, draw_leak, 0) &&
    tex_render(engine, TEX_LEAK_WALK_1, draw_leak, 1) &&
    tex_render(engine, TEX_LEAK_ATK, draw_leak, 2) &&
    tex_render(engine, TEX_LEAK_DEAD, draw_leak, 3) &&
    tex_render(engine, TEX_RACE_WALK_0, draw_race, 0) &&
    tex_render(engine, TEX_RACE_WALK_1, draw_race, 1) &&
    tex_render(engine, TEX_RACE_ATK, draw_race, 2) &&
    tex_render(engine, TEX_RACE_DEAD, draw_race, 3) &&
    tex_render(engine, TEX_DFREE_WALK_0, draw_dfree, 0) &&
    tex_render(engine, TEX_DFREE_WALK_1, draw_dfree, 1) &&
    tex_render(engine, TEX_DFREE_ATK, draw_dfree, 2) &&
    tex_render(engine, TEX_DFREE_DEAD, draw_dfree, 3) &&
    tex_render(engine, TEX_FREE1_WALK_0, draw_free1, 0) &&
    tex_render(engine, TEX_FREE1_WALK_1, draw_free1, 1) &&
    tex_render(engine, TEX_FREE1_ATK, draw_free1, 2) &&
    tex_render(engine, TEX_FREE1_DEAD, draw_free1, 3) &&
    tex_render(engine, TEX_BOSS_WALK_0, draw_boss, 0) &&
    tex_render(engine, TEX_BOSS_WALK_1, draw_boss, 1) &&
    tex_render(engine, TEX_BOSS_ATK, draw_boss, 2) &&
    tex_render(engine, TEX_BOSS_DEAD, draw_boss, 3) &&
    tex_render(engine, TEX_THOMAS_0, draw_thomas, 0) &&
    tex_render(engine, TEX_THOMAS_1, draw_thomas, 1) &&
    tex_render(engine, TEX_ITEM_COFFEE, draw_item_coffee, 0) &&
    tex_render(engine, TEX_ITEM_AMMO, draw_item_ammo, 0) &&
    tex_render(engine, TEX_ITEM_FACT, draw_item_fact, 0) &&
    tex_render(engine, TEX_ITEM_GDB, draw_item_gdb, 0) &&
    tex_render(engine, TEX_ITEM_ASAN, draw_item_asan, 0) &&
    tex_render(engine, TEX_ITEM_ARMOR, draw_item_armor, 0) &&
    tex_render(engine, TEX_PROJ, draw_proj, 0) &&
    tex_render(engine, TEX_BPROJ, draw_bproj, 0);
}

static void draw_boss (cairo_t *cr, int p)
{
  double x;
  int i;
  if (p == 3) {
    tex_puddle(cr, "#3a0a18", 30, "err: locked", "#9f9");
    return;
  }
  tex_source(cr, "#5a1030");
  cairo_set_line_width(cr, 3);
  for (i = 0; i < 8; i++) {
    x = 6 + i * 7.5;
    cairo_new_path(cr);
    cairo_move_to(cr, x, 40);
    tex_quadratic_to(cr,
                     x + (p == 1 ? 6 : -6) * (i % 2 ? 1 : -1), 52,
                     x + (i % 2 ? 4 : -4), 63);
    cairo_stroke(cr);
  }
  tex_source(cr, p == 2 ? "#b0204a" : "#7a1438");
  cairo_new_path(cr);
  tex_ellipse_path(cr, 32, 29, 28, 24);
  cairo_fill(cr);
  tex_source(cr, "#fff");
  cairo_new_path(cr);
  tex_ellipse_path(cr, 32, 22, 12, 9);
  cairo_fill(cr);
  tex_source(cr, p == 2 ? "#f00" : "#200");
  cairo_new_path(cr);
  cairo_arc(cr, 32, 22, 5, 0, TEX_2PI);
  cairo_fill(cr);
  tex_source(cr, "#fff");
  for (i = 0; i < 7; i++) {
    cairo_new_path(cr);
    cairo_move_to(cr, 14 + i * 5, 37);
    cairo_line_to(cr, 16.5 + i * 5, p == 2 ? 47 : 43);
    cairo_line_to(cr, 19 + i * 5, 37);
    cairo_fill(cr);
  }
  tex_txt(cr, "env->err", 32, 9, "bold 9px monospace", "#ffd23a",
          'c');
}

static void draw_dfree (cairo_t *cr, int p)
{
  double x;
  int i;
  int o;
  if (p == 3) {
    tex_source(cr, "#8a4a10");
    cairo_new_path(cr);
    tex_ellipse_path(cr, 20, 58, 14, 5);
    tex_ellipse_path(cr, 44, 58, 14, 5);
    cairo_fill(cr);
    return;
  }
  for (i = 0; i < 2; i++) {
    x = i == 0 ? 20 : 44;
    o = ((i == 0) == (p == 1)) ? 2 : 0;
    tex_frect(cr, "#111", x - 6, 46 + o, 3, 17 - o);
    tex_frect(cr, "#111", x + 3, 46 + o, 3, 17 - o);
    tex_source(cr, p == 2 ? "#ffb040" : "#e8841c");
    cairo_new_path(cr);
    cairo_arc(cr, x, 34 + o, 15, 0, TEX_2PI);
    cairo_fill(cr);
    tex_frect(cr, "#fff", x - 7, 25 + o, 5, 5);
    tex_frect(cr, "#fff", x + 2, 25 + o, 5, 5);
    tex_frect(cr, "#000", x - 5, 27 + o, 2, 2);
    tex_frect(cr, "#000", x + 4, 27 + o, 2, 2);
    tex_txt(cr, "free()", x, 39 + o, "bold 7px monospace",
            "#401a00", 'c');
  }
}

static void draw_floor_carpet (cairo_t *cr, int p)
{
  (void) p;
  tex_frect(cr, "#1c2440", 0, 0, 64, 64);
  tex_speckle(cr, "#4a5a9a", 900, 0.6);
  tex_frect(cr, "#c83", 0, 30, 64, 2);
}

static void draw_floor_dark (cairo_t *cr, int p)
{
  (void) p;
  tex_frect(cr, "#16181c", 0, 0, 64, 64);
  tex_frect(cr, "#24272d", 2, 2, 28, 28);
  tex_frect(cr, "#24272d", 34, 34, 28, 28);
  tex_frect(cr, "#a33", 30, 30, 4, 4);
}

static void draw_floor_grate (cairo_t *cr, int p)
{
  int i;
  (void) p;
  tex_frect(cr, "#1a1c1f", 0, 0, 64, 64);
  tex_source(cr, "#4a4f57");
  for (i = 0; i < 64; i += 8) {
    cairo_rectangle(cr, i, 0, 3, 64);
    cairo_fill(cr);
    cairo_rectangle(cr, 0, i, 64, 3);
    cairo_fill(cr);
  }
  tex_speckle(cr, "#000", 300, 0.4);
}

static void draw_floor_panel (cairo_t *cr, int p)
{
  (void) p;
  tex_frect(cr, "#5c5f66", 0, 0, 64, 64);
  tex_frect(cr, "#44474d", 0, 0, 64, 2);
  tex_frect(cr, "#44474d", 0, 0, 2, 64);
  tex_frect(cr, "#e8e0b0", 26, 26, 12, 12);
  tex_speckle(cr, "#000", 200, 0.3);
}

static void draw_floor_tile (cairo_t *cr, int p)
{
  int x;
  int y;
  (void) p;
  for (y = 0; y < 2; y++)
    for (x = 0; x < 2; x++)
      tex_frect(cr, (x + y) % 2 ? "#2e3136" : "#383b41",
                x * 32, y * 32, 32, 32);
  tex_srect(cr, "#1d1f23", 0.5, 0.5, 63, 63);
  tex_speckle(cr, "#000", 500, 0.3);
}

static void draw_free1 (cairo_t *cr, int p)
{
  int o;
  if (p == 3) {
    tex_puddle(cr, "#8a4a10", 18, "NULL", "#fda");
    return;
  }
  o = p == 1 ? 2 : 0;
  tex_frect(cr, "#111", 24, 46 + o, 5, 17 - o);
  tex_frect(cr, "#111", 35, 46 + o, 5, 17 - o);
  tex_source(cr, p == 2 ? "#ffb040" : "#e8841c");
  cairo_new_path(cr);
  cairo_arc(cr, 32, 30 + o, 22, 0, TEX_2PI);
  cairo_fill(cr);
  tex_frect(cr, "#fff", 20, 18 + o, 9, 8);
  tex_frect(cr, "#fff", 35, 18 + o, 9, 8);
  tex_frect(cr, "#000", 23, 21 + o, 3, 3);
  tex_frect(cr, "#000", 38, 21 + o, 3, 3);
  tex_txt(cr, "free()", 32, 38 + o, "bold 10px monospace",
          "#401a00", 'c');
}

static void draw_item_ammo (cairo_t *cr, int p)
{
  (void) p;
  tex_frect(cr, "#2d4a22", 6, 24, 52, 38);
  tex_frect(cr, "#3f6a30", 6, 24, 52, 8);
  cairo_set_line_width(cr, 1);
  tex_srect(cr, "#18280f", 6.5, 24.5, 51, 37);
  tex_txt(cr, "BRK", 32, 47, "bold 15px monospace", "#e8d040",
          'c');
}

static void draw_item_armor (cairo_t *cr, int p)
{
  (void) p;
  tex_source(cr, "#1e8a3a");
  cairo_new_path(cr);
  cairo_move_to(cr, 32, 4);
  cairo_line_to(cr, 58, 12);
  cairo_line_to(cr, 54, 40);
  cairo_line_to(cr, 32, 62);
  cairo_line_to(cr, 10, 40);
  cairo_line_to(cr, 6, 12);
  cairo_close_path(cr);
  cairo_fill(cr);
  tex_source(cr, "#bfffcf");
  cairo_set_line_width(cr, 5);
  cairo_new_path(cr);
  cairo_move_to(cr, 20, 30);
  cairo_line_to(cr, 29, 40);
  cairo_line_to(cr, 45, 20);
  cairo_stroke(cr);
  tex_txt(cr, "tests", 32, 52, "bold 9px monospace", "#fff", 'c');
}

static void draw_item_asan (cairo_t *cr, int p)
{
  int i;
  (void) p;
  tex_frect(cr, "#50545c", 8, 26, 48, 22);
  tex_source(cr, "#2a2c30");
  for (i = 0; i < 4; i++) {
    cairo_rectangle(cr, 2, 28 + i * 5, 14, 3);
    cairo_fill(cr);
  }
  tex_frect(cr, "#c22", 24, 26, 12, 22);
  tex_txt(cr, "ASan", 32, 58, "bold 12px monospace", "#f55", 'c');
}

static void draw_item_coffee (cairo_t *cr, int p)
{
  (void) p;
  tex_source(cr, "rgba(255,255,255,.6)");
  cairo_set_line_width(cr, 3);
  cairo_new_path(cr);
  cairo_move_to(cr, 24, 20);
  tex_quadratic_to(cr, 18, 12, 26, 4);
  cairo_move_to(cr, 36, 20);
  tex_quadratic_to(cr, 30, 12, 38, 4);
  cairo_stroke(cr);
  tex_source(cr, "#eee");
  cairo_set_line_width(cr, 5);
  cairo_new_path(cr);
  cairo_arc(cr, 46, 42, 8, -1.4, 1.4);
  cairo_stroke(cr);
  tex_frect(cr, "#eee", 12, 24, 34, 38);
  tex_frect(cr, "#5a3212", 14, 24, 30, 5);
  tex_txt(cr, "kmx", 29, 45, "bold 11px monospace", "#f0a030",
          'c');
}

static void draw_item_fact (cairo_t *cr, int p)
{
  (void) p;
  tex_source(cr, "rgba(60,220,255,.3)");
  cairo_new_path(cr);
  cairo_arc(cr, 32, 32, 30, 0, TEX_2PI);
  cairo_fill(cr);
  tex_txt(cr, "{ }", 32, 26, "bold 22px monospace", "#8ef", 'c');
  tex_txt(cr, "fact", 32, 48, "bold 11px monospace", "#fff", 'c');
}

static void draw_item_gdb (cairo_t *cr, int p)
{
  (void) p;
  tex_frect(cr, "#555a62", 2, 30, 44, 5);
  tex_frect(cr, "#555a62", 2, 37, 44, 5);
  tex_frect(cr, "#6b3a1a", 24, 28, 14, 16);
  tex_frect(cr, "#6b3a1a", 40, 33, 22, 14);
  tex_txt(cr, "gdb", 32, 56, "bold 13px monospace", "#ffd23a",
          'c');
}

static void draw_leak (cairo_t *cr, int p)
{
  const char *c;
  int d;
  if (p == 3) {
    tex_puddle(cr, "#1b6f6a", 28, "free()", "#cff");
    return;
  }
  c = p == 2 ? "#3fe0d0" : "#22a49a";
  tex_source(cr, c);
  cairo_new_path(cr);
  cairo_move_to(cr, 32, 4);
  cairo_curve_to(cr, 44, 20, 56, 32, 54, 44);
  cairo_curve_to(cr, 52, 58, 12, 58, 10, 44);
  cairo_curve_to(cr, 8, 32, 20, 20, 32, 4);
  cairo_fill(cr);
  tex_frect(cr, "rgba(255,255,255,.35)", 19, 26, 4, 10);
  d = p ? 6 : 0;
  tex_frect(cr, c, 16, 54, 4, 4 + d);
  tex_frect(cr, c, 30, 55, 5, 8 - d);
  tex_frect(cr, c, 44, 53, 4, 3 + d);
  tex_frect(cr, "#fff", 22, 29, 7, 7);
  tex_frect(cr, "#fff", 35, 29, 7, 7);
  tex_frect(cr, "#000", 24, 32, 3, 3);
  tex_frect(cr, "#000", 37, 32, 3, 3);
  tex_frect(cr, "#023", 25, 39, 14, p == 2 ? 6 : 2);
  tex_txt(cr, "LEAK", 32, 50, "bold 9px monospace", "#063", 'c');
}

static void draw_proj (cairo_t *cr, int p)
{
  cairo_pattern_t *pat;
  (void) p;
  pat = cairo_pattern_create_radial(32, 32, 2, 32, 32, 30);
  cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 1.0, 1.0, 1.0);
  cairo_pattern_add_color_stop_rgba(pat, 0.3, 1.0, 0.8, 0.25, 1.0);
  cairo_pattern_add_color_stop_rgba(pat, 0.7, 1.0, 80.0 / 255, 0.0,
                                    0.8);
  cairo_pattern_add_color_stop_rgba(pat, 1.0, 1.0, 0.0, 0.0, 0.0);
  cairo_set_source(cr, pat);
  cairo_rectangle(cr, 0, 0, 64, 64);
  cairo_fill(cr);
  cairo_pattern_destroy(pat);
  tex_txt(cr, "0x0", 32, 32, "bold 12px monospace", "#600", 'c');
}

static void draw_bproj (cairo_t *cr, int p)
{
  cairo_pattern_t *pat;
  (void) p;
  pat = cairo_pattern_create_radial(32, 32, 2, 32, 32, 30);
  cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 1.0, 1.0, 1.0);
  cairo_pattern_add_color_stop_rgba(pat, 0.3, 1.0, 0.376, 0.816,
                                    1.0);
  cairo_pattern_add_color_stop_rgba(pat, 0.7, 160.0 / 255, 0.0, 1.0,
                                    0.8);
  cairo_pattern_add_color_stop_rgba(pat, 1.0, 80.0 / 255, 0.0,
                                    160.0 / 255, 0.0);
  cairo_set_source(cr, pat);
  cairo_rectangle(cr, 0, 0, 64, 64);
  cairo_fill(cr);
  cairo_pattern_destroy(pat);
  tex_txt(cr, "err", 32, 32, "bold 12px monospace", "#300", 'c');
}

static void draw_race (cairo_t *cr, int p)
{
  if (p == 3) {
    tex_puddle(cr, "#5a2a80", 22, "mutex", "#fdf");
    return;
  }
  tex_ghost(cr, 24 + (p == 1 ? -3 : 0), "rgba(170,90,255,.55)",
            p & 1);
  tex_ghost(cr, 38 + (p == 1 ? 3 : 0),
            p == 2 ? "#e070ff" : "#a050e8", p & 1);
  tex_txt(cr, "RACE", 38, 40, "bold 8px monospace", "#fff", 'c');
}

static void draw_segv (cairo_t *cr, int p)
{
  double l;
  if (p == 3) {
    tex_puddle(cr, "#7a1010", 24, "0xe8", "#fc3");
    return;
  }
  tex_source(cr, "#111");
  cairo_set_line_width(cr, 3);
  l = p == 1 ? 4 : -4;
  cairo_new_path(cr);
  cairo_move_to(cr, 26, 44);
  cairo_line_to(cr, 24 + l, 63);
  cairo_move_to(cr, 38, 44);
  cairo_line_to(cr, 40 - l, 63);
  cairo_stroke(cr);
  tex_oct(cr, 32, 28, 22, "#fff");
  tex_oct(cr, 32, 28, 19, p == 2 ? "#ff3020" : "#c81818");
  tex_frect(cr, "#fff", 22, 15, 7, 6);
  tex_frect(cr, "#fff", 35, 15, 7, 6);
  tex_frect(cr, "#000", 25, 17, 3, 3);
  tex_frect(cr, "#000", 36, 17, 3, 3);
  tex_source(cr, "#000");
  cairo_set_line_width(cr, 2);
  cairo_new_path(cr);
  cairo_move_to(cr, 20, 11);
  cairo_line_to(cr, 29, 15);
  cairo_move_to(cr, 44, 11);
  cairo_line_to(cr, 35, 15);
  cairo_stroke(cr);
  tex_txt(cr, "SEGV", 32, 30, "bold 10px monospace", "#fff", 'c');
  if (p == 2)
    tex_frect(cr, "#ffe040", 25, 37, 14, 5);
  else
    tex_frect(cr, "#400", 26, 39, 12, 2);
}

static void draw_thomas (cairo_t *cr, int p)
{
  tex_frect(cr, "#2b3a5a", 24, 44, 7, 20);
  tex_frect(cr, "#2b3a5a", 33, 44, 7, 20);
  tex_frect(cr, "#1d2027", 20, 22, 24, 24);
  tex_frect(cr, "#1d2027", 14, 24, 6, 16);
  if (p)
    tex_frect(cr, "#1d2027", 44, 10, 6, 14);
  else
    tex_frect(cr, "#1d2027", 44, 24, 6, 16);
  tex_frect(cr, "#e0b090", 14, 39, 6, 4);
  tex_frect(cr, "#e0b090", 44, p ? 6 : 39, 6, 4);
  tex_txt(cr, "kmx", 32, 31, "bold 9px monospace", "#f0a030", 'c');
  tex_source(cr, "#e0b090");
  cairo_new_path(cr);
  cairo_arc(cr, 32, 12, 10, 0, TEX_2PI);
  cairo_fill(cr);
  tex_frect(cr, "#3a2a1a", 22, 2, 20, 5);
  tex_frect(cr, "#000", 27, 11, 2, 2);
  tex_frect(cr, "#000", 35, 11, 2, 2);
  tex_frect(cr, "#000", 28, 17, 8, 1);
}

static void draw_wall_1 (cairo_t *cr, int p)
{
  static const double bolts[8][2] = {
    {4, 4}, {58, 4}, {4, 27}, {58, 27},
    {4, 36}, {58, 36}, {4, 58}, {58, 58}
  };
  int i;
  (void) p;
  tex_frect(cr, "#141b27", 0, 0, 64, 64);
  tex_frect(cr, "#223049", 1, 1, 62, 30);
  tex_frect(cr, "#223049", 1, 33, 62, 30);
  tex_speckle(cr, "#000", 400, 0.5);
  tex_source(cr, "#5a6b85");
  for (i = 0; i < 8; i++) {
    cairo_rectangle(cr, bolts[i][0], bolts[i][1], 2, 2);
    cairo_fill(cr);
  }
  tex_txt(cr, "kmx", 32, 17, "bold 20px monospace", "#f0a030",
          'c');
  tex_txt(cr, ".io", 32, 47, "bold 13px monospace", "#8fb0d8",
          'c');
}

static void draw_wall_2 (cairo_t *cr, int p)
{
  static const char *const lines[12] = {
    "s_tag *t;", "env_eval(", "buf_parse", "if (! r)",
    "return 0;", "free(p);", "tag_init", "err_puts(",
    "rwlock_r", "}", "marshall", "facts_add"
  };
  const char *col;
  double x;
  int i;
  int idx;
  (void) p;
  tex_frect(cr, "#050805", 0, 0, 64, 64);
  for (i = 0; i < 9; i++) {
    idx = (int) (tex_rnd() * 12);
    x = 2 + (int) (tex_rnd() * 6);
    col = tex_rnd() < 0.15 ? "#e0c050" : "#3fd66a";
    tex_txt(cr, lines[idx], x, 4 + i * 7, "7px monospace", col,
            'l');
  }
  cairo_set_line_width(cr, 1);
  tex_srect(cr, "#1a2a1a", 0.5, 0.5, 63, 63);
}

static void draw_wall_3 (cairo_t *cr, int p)
{
  double a;
  int i;
  (void) p;
  tex_frect(cr, "#b8902a", 0, 0, 64, 64);
  tex_frect(cr, "#d4ab3c", 2, 2, 60, 60);
  tex_speckle(cr, "#5a4010", 300, 0.6);
  tex_source(cr, "#3a2a08");
  cairo_set_line_width(cr, 2);
  for (i = 0; i < 12; i++) {
    a = i / 12.0 * TEX_2PI;
    cairo_new_path(cr);
    cairo_move_to(cr, 32 + cos(a) * 13, 26 + sin(a) * 13);
    cairo_line_to(cr, 32 + cos(a) * 18, 26 + sin(a) * 18);
    cairo_stroke(cr);
  }
  tex_source(cr, "#f5d860");
  cairo_new_path(cr);
  cairo_move_to(cr, 20, 26);
  cairo_line_to(cr, 10, 18);
  cairo_line_to(cr, 10, 34);
  cairo_fill(cr);
  cairo_new_path(cr);
  cairo_arc(cr, 32, 26, 13, 0, TEX_2PI);
  cairo_fill_preserve(cr);
  tex_source(cr, "#3a2a08");
  cairo_stroke(cr);
  tex_source(cr, "#fff");
  cairo_new_path(cr);
  cairo_arc(cr, 38, 22, 4, 0, TEX_2PI);
  cairo_fill(cr);
  tex_frect(cr, "#000", 39, 21, 2, 3);
  tex_txt(cr, "OpenBSD", 32, 54, "bold 9px monospace", "#2a1c04",
          'c');
}

static void draw_wall_4 (cairo_t *cr, int p)
{
  (void) p;
  tex_rack(cr, "OVH");
}

static void draw_wall_5 (cairo_t *cr, int p)
{
  static const int rgb[3] = {150, 30, 24};
  (void) p;
  tex_bricks(cr, rgb, "#2a0a08");
  tex_frect(cr, "rgba(0,0,0,.55)", 0, 20, 64, 24);
  tex_txt(cr, "SIGSEGV", 32, 28, "bold 11px monospace", "#ffd23a",
          'c');
  tex_txt(cr, "core dumped", 32, 38, "6px monospace", "#fff", 'c');
}

static void draw_wall_6 (cairo_t *cr, int p)
{
  static const int rgb[3] = {110, 112, 118};
  (void) p;
  tex_bricks(cr, rgb, "#2c2d30");
}

static void draw_wall_d (cairo_t *cr, int p)
{
  int i;
  (void) p;
  tex_frect(cr, "#4d525b", 0, 0, 64, 64);
  tex_frect(cr, "#626873", 3, 8, 27, 48);
  tex_frect(cr, "#626873", 34, 8, 27, 48);
  tex_frect(cr, "#1c1e22", 31, 8, 2, 48);
  for (i = 0; i < 8; i++) {
    tex_frect(cr, i % 2 ? "#111" : "#e8b820", i * 8, 0, 8, 6);
    tex_frect(cr, i % 2 ? "#111" : "#e8b820", i * 8, 58, 8, 6);
  }
  tex_txt(cr, "[E]", 32, 32, "bold 10px monospace", "#e8b820",
          'c');
}

static void draw_wall_x (cairo_t *cr, int p)
{
  (void) p;
  tex_frect(cr, "#2b2e33", 0, 0, 64, 64);
  tex_frect(cr, "#000", 5, 6, 54, 40);
  tex_txt(cr, "$ git push", 8, 14, "bold 7px monospace", "#3f6",
          'l');
  tex_txt(cr, "origin", 8, 23, "7px monospace", "#3f6", 'l');
  tex_txt(cr, "master", 8, 31, "7px monospace", "#3f6", 'l');
  tex_frect(cr, "#3f6", 8, 37, 5, 6);
  tex_txt(cr, "EXIT [E]", 32, 54, "bold 8px monospace", "#f33",
          'c');
}

static bool tex_color_parse (const char *str, double *r, double *g,
                             double *b, double *a)
{
  const char *p;
  double af;
  int bi;
  int gi;
  int ri;
  unsigned int v;
  *a = 1.0;
  if (str[0] == '#') {
    if (strlen(str) == 4) {
      p = str + 1;
      ri = tex_hex(p[0]);
      gi = tex_hex(p[1]);
      bi = tex_hex(p[2]);
      if (ri < 0 || gi < 0 || bi < 0)
        return false;
      *r = ri / 15.0;
      *g = gi / 15.0;
      *b = bi / 15.0;
      return true;
    }
    if (strlen(str) == 7) {
      v = (unsigned int) strtoul(str + 1, NULL, 16);
      *r = ((v >> 16) & 0xff) / 255.0;
      *g = ((v >> 8) & 0xff) / 255.0;
      *b = (v & 0xff) / 255.0;
      return true;
    }
    return false;
  }
  if (! strncmp(str, "rgba(", 5)) {
    if (sscanf(str, "rgba(%d,%d,%d,%lf)", &ri, &gi, &bi, &af) != 4)
      return false;
    *r = ri / 255.0;
    *g = gi / 255.0;
    *b = bi / 255.0;
    *a = af;
    return true;
  }
  if (! strncmp(str, "rgb(", 4)) {
    if (sscanf(str, "rgb(%d,%d,%d)", &ri, &gi, &bi) != 3)
      return false;
    *r = ri / 255.0;
    *g = gi / 255.0;
    *b = bi / 255.0;
    return true;
  }
  return false;
}

static void tex_bricks (cairo_t *cr, const int rgb[3],
                        const char *mort)
{
  double k;
  int c;
  int o;
  int r;
  tex_frect(cr, mort, 0, 0, 64, 64);
  for (r = 0; r < 8; r++) {
    o = r % 2 ? 8 : 0;
    for (c = -1; c < 4; c++) {
      k = 0.8 + tex_rnd() * 0.35;
      tex_source_rgb(cr, (int) (rgb[0] * k), (int) (rgb[1] * k),
                     (int) (rgb[2] * k));
      cairo_rectangle(cr, c * 16 + o + 1, r * 8 + 1, 15, 7);
      cairo_fill(cr);
    }
  }
  tex_speckle(cr, "#000", 300, 0.4);
}

static void tex_ellipse_path (cairo_t *cr, double x, double y,
                              double rx, double ry)
{
  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, rx, ry);
  cairo_arc(cr, 0, 0, 1, 0, TEX_2PI);
  cairo_restore(cr);
}

static void tex_frect (cairo_t *cr, const char *col, double x,
                       double y, double w, double h)
{
  tex_source(cr, col);
  cairo_rectangle(cr, x, y, w, h);
  cairo_fill(cr);
}

static int tex_hex (char c)
{
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

static void tex_ghost (cairo_t *cr, double x, const char *c, int p)
{
  int i;
  tex_source(cr, c);
  cairo_new_path(cr);
  cairo_arc(cr, x, 22, 15, TEX_PI, TEX_2PI);
  cairo_line_to(cr, x + 15, 58);
  for (i = 1; i <= 5; i++)
    cairo_line_to(cr, x + 15 - i * 6, (i + p) % 2 ? 51 : 58);
  cairo_close_path(cr);
  cairo_fill(cr);
  tex_frect(cr, "#fff", x - 8, 17, 6, 7);
  tex_frect(cr, "#fff", x + 2, 17, 6, 7);
  tex_frect(cr, "#20a", x - 6, 19, 3, 4);
  tex_frect(cr, "#20a", x + 4, 19, 3, 4);
}

static void tex_oct (cairo_t *cr, double x, double y, double r,
                     const char *c)
{
  double a;
  int i;
  tex_source(cr, c);
  cairo_new_path(cr);
  for (i = 0; i < 8; i++) {
    a = TEX_PI / 8 + i * TEX_PI / 4;
    cairo_line_to(cr, x + cos(a) * r, y + sin(a) * r);
  }
  cairo_fill(cr);
}

static void tex_puddle (cairo_t *cr, const char *c, double w,
                        const char *label, const char *lc)
{
  tex_source(cr, c);
  cairo_new_path(cr);
  tex_ellipse_path(cr, 32, 58, w, 6);
  cairo_fill(cr);
  tex_txt(cr, label, 32, 57, "bold 7px monospace", lc, 'c');
}

static void tex_quadratic_to (cairo_t *cr, double x1, double y1,
                              double x2, double y2)
{
  double x0;
  double y0;
  cairo_get_current_point(cr, &x0, &y0);
  cairo_curve_to(cr,
                 x0 + 2.0 / 3.0 * (x1 - x0),
                 y0 + 2.0 / 3.0 * (y1 - y0),
                 x2 + 2.0 / 3.0 * (x1 - x2),
                 y2 + 2.0 / 3.0 * (y1 - y2),
                 x2, y2);
}

static void tex_rack (cairo_t *cr, const char *label)
{
  double y;
  int i;
  int v;
  tex_frect(cr, "#202226", 0, 0, 64, 64);
  for (i = 0; i < 7; i++) {
    y = 2 + i * 8;
    tex_frect(cr, "#3b3f46", 4, y, 56, 7);
    tex_source(cr, "#2a2d33");
    for (v = 0; v < 6; v++) {
      cairo_rectangle(cr, 22 + v * 4, y + 2, 2, 3);
      cairo_fill(cr);
    }
    tex_frect(cr, tex_rnd() < 0.7 ? "#3f3" : "#fa3",
              7, y + 2, 2, 2);
    tex_frect(cr, tex_rnd() < 0.5 ? "#3f3" : "#333",
              11, y + 2, 2, 2);
    tex_frect(cr, "#8a8f98", 52, y + 2, 5, 3);
  }
  tex_txt(cr, label, 32, 60, "bold 7px monospace", "#9aa", 'c');
}

static bool tex_render (s_engine *engine, int id,
                        void (*draw) (cairo_t *cr, int p), int p)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  unsigned char *data;
  int stride;
  int x;
  int y;
  uint32_t a;
  uint32_t b;
  uint32_t g;
  uint32_t px;
  uint32_t r;
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 64, 64);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    cairo_surface_destroy(surface);
    return false;
  }
  cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return false;
  }
  cairo_set_line_width(cr, 1);
  draw(cr, p);
  cairo_surface_flush(surface);
  data = cairo_image_surface_get_data(surface);
  stride = cairo_image_surface_get_stride(surface);
  for (y = 0; y < 64; y++) {
    for (x = 0; x < 64; x++) {
      memcpy(&px, data + y * stride + x * 4, 4);
      a = px >> 24;
      r = (px >> 16) & 0xff;
      g = (px >> 8) & 0xff;
      b = px & 0xff;
      if (a) {
        r = r * 255 / a;
        g = g * 255 / a;
        b = b * 255 / a;
        if (r > 255)
          r = 255;
        if (g > 255)
          g = 255;
        if (b > 255)
          b = 255;
      }
      else {
        r = 0;
        g = 0;
        b = 0;
      }
      engine->tex[id][y * 64 + x] =
        (a << 24) | (r << 16) | (g << 8) | b;
    }
  }
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  return true;
}

static double tex_rnd (void)
{
  uint32_t t;
  g_seed = g_seed + 0x6D2B79F5u;
  t = (g_seed ^ (g_seed >> 15)) * (1u | g_seed);
  t = (t + (t ^ (t >> 7)) * (61u | t)) ^ t;
  return (double) (t ^ (t >> 14)) / 4294967296.0;
}

static void tex_source (cairo_t *cr, const char *col)
{
  double a;
  double b;
  double g;
  double r;
  if (! tex_color_parse(col, &r, &g, &b, &a))
    return;
  cairo_set_source_rgba(cr, r, g, b, a);
}

static void tex_source_rgb (cairo_t *cr, int r, int g, int b)
{
  if (r > 255)
    r = 255;
  if (g > 255)
    g = 255;
  if (b > 255)
    b = 255;
  cairo_set_source_rgb(cr, r / 255.0, g / 255.0, b / 255.0);
}

static void tex_speckle (cairo_t *cr, const char *col, int n,
                         double a)
{
  double alpha;
  double b;
  double ca;
  double g;
  double r;
  double x;
  double y;
  int i;
  if (! tex_color_parse(col, &r, &g, &b, &ca))
    return;
  for (i = 0; i < n; i++) {
    alpha = a * tex_rnd();
    x = (double) (int) (tex_rnd() * 64);
    y = (double) (int) (tex_rnd() * 64);
    cairo_set_source_rgba(cr, r, g, b, ca * alpha);
    cairo_rectangle(cr, x, y, 1, 1);
    cairo_fill(cr);
  }
}

static void tex_srect (cairo_t *cr, const char *col, double x,
                       double y, double w, double h)
{
  tex_source(cr, col);
  cairo_rectangle(cr, x, y, w, h);
  cairo_stroke(cr);
}

static void tex_txt (cairo_t *cr, const char *s, double x, double y,
                     const char *font, const char *col, char align)
{
  cairo_font_weight_t weight;
  cairo_text_extents_t ext;
  const char *p;
  double size;
  p = font;
  weight = CAIRO_FONT_WEIGHT_NORMAL;
  if (! strncmp(p, "bold ", 5)) {
    weight = CAIRO_FONT_WEIGHT_BOLD;
    p += 5;
  }
  size = strtod(p, NULL);
  cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL,
                         weight);
  cairo_set_font_size(cr, size);
  cairo_text_extents(cr, s, &ext);
  if (align == 'c')
    x -= ext.x_advance / 2;
  else if (align == 'r')
    x -= ext.x_advance;
  tex_source(cr, col);
  cairo_move_to(cr, x, y - (ext.y_bearing + ext.height / 2));
  cairo_show_text(cr, s);
}
