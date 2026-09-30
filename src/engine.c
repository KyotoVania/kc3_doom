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
#include <string.h>
#include "engine.h"

typedef struct engine_sprite_order {
  double d;
  int    i;
} s_engine_sprite_order;

static uint32_t engine_bright (uint32_t c)
{
  return ((c >> 1) & 0x7f7f7f) + 0x808080;
}

static bool engine_char_solid (char c)
{
  return (c == '1' || c == '2' || c == '3' || c == '4' ||
          c == '5' || c == '6' || c == 'D' || c == 'X');
}

static int engine_fog (double d, double fog)
{
  double f;
  f = 1.0 - d / fog;
  if (f < 0.16)
    f = 0.16;
  return (int) (f * 256);
}

static uint32_t engine_shade (uint32_t c, int f)
{
  return ((((c >> 16) & 0xff) * f >> 8) << 16) |
         ((((c >> 8) & 0xff) * f >> 8) << 8) |
         ((c & 0xff) * f >> 8);
}

static void engine_sprite_sort (s_engine_sprite_order *order, int n)
{
  s_engine_sprite_order tmp;
  int i;
  int j;
  for (i = 1; i < n; i++) {
    tmp = order[i];
    for (j = i; j > 0 && order[j - 1].d < tmp.d; j--)
      order[j] = order[j - 1];
    order[j] = tmp;
  }
}

static const uint32_t * engine_wall_tex (const s_engine *engine, char c)
{
  switch (c) {
  case '1': return engine->tex[TEX_WALL_1];
  case '2': return engine->tex[TEX_WALL_2];
  case '3': return engine->tex[TEX_WALL_3];
  case '4': return engine->tex[TEX_WALL_4];
  case '5': return engine->tex[TEX_WALL_5];
  case '6': return engine->tex[TEX_WALL_6];
  case 'D': return engine->tex[TEX_WALL_D];
  case 'X': return engine->tex[TEX_WALL_X];
  }
  return engine->tex[TEX_WALL_6];
}

bool engine_init (s_engine *engine)
{
  memset(engine, 0, sizeof(*engine));
  if (cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, ENGINE_W) !=
      ENGINE_W * 4)
    return false;
  engine->surface =
    cairo_image_surface_create_for_data((unsigned char *) engine->frame,
                                        CAIRO_FORMAT_RGB24,
                                        ENGINE_W, ENGINE_VH,
                                        ENGINE_W * 4);
  if (cairo_surface_status(engine->surface) != CAIRO_STATUS_SUCCESS) {
    cairo_surface_destroy(engine->surface);
    engine->surface = NULL;
    return false;
  }
  return true;
}

void engine_clean (s_engine *engine)
{
  if (engine->surface) {
    cairo_surface_destroy(engine->surface);
    engine->surface = NULL;
  }
}

bool engine_grid_load (s_engine *engine, const char **rows, int h)
{
  size_t w;
  int y;
  if (h < 0 || h > ENGINE_GRID_MAX)
    return false;
  w = h ? strlen(rows[0]) : 0;
  if (w > ENGINE_GRID_MAX)
    return false;
  for (y = 1; y < h; y++)
    if (strlen(rows[y]) != w)
      return false;
  engine->grid_w = (int) w;
  engine->grid_h = h;
  memset(engine->grid, 0, sizeof(engine->grid));
  for (y = 0; y < h; y++)
    memcpy(engine->grid[y], rows[y], w);
  return true;
}

bool engine_grid_set (s_engine *engine, int x, int y, char c)
{
  if (x < 0 || y < 0 || x >= engine->grid_w || y >= engine->grid_h)
    return false;
  engine->grid[y][x] = c;
  return true;
}

char engine_grid_get (const s_engine *engine, int x, int y)
{
  if (x < 0 || y < 0 || x >= engine->grid_w || y >= engine->grid_h)
    return 0;
  return engine->grid[y][x];
}

bool engine_solid (const s_engine *engine, int x, int y)
{
  if (x < 0 || y < 0 || x >= engine->grid_w || y >= engine->grid_h)
    return true;
  return engine_char_solid(engine->grid[y][x]);
}

s_engine_hit engine_cast (const s_engine *engine, double x, double y,
                          double dx, double dy)
{
  s_engine_hit hit;
  double ddx;
  double ddy;
  double sdx;
  double sdy;
  int i;
  int mx;
  int my;
  int sx;
  int sy;
  mx = (int) x;
  my = (int) y;
  ddx = dx == 0.0 ? 1e30 : fabs(1.0 / dx);
  ddy = dy == 0.0 ? 1e30 : fabs(1.0 / dy);
  hit.side = 0;
  if (dx < 0) {
    sx = -1;
    sdx = (x - mx) * ddx;
  }
  else {
    sx = 1;
    sdx = (mx + 1 - x) * ddx;
  }
  if (dy < 0) {
    sy = -1;
    sdy = (y - my) * ddy;
  }
  else {
    sy = 1;
    sdy = (my + 1 - y) * ddy;
  }
  for (i = 0; i < 96; i++) {
    if (sdx < sdy) {
      sdx += ddx;
      mx += sx;
      hit.side = 0;
    }
    else {
      sdy += ddy;
      my += sy;
      hit.side = 1;
    }
    if (engine_solid(engine, mx, my)) {
      hit.d = hit.side ? sdy - ddy : sdx - ddx;
      hit.mx = mx;
      hit.my = my;
      return hit;
    }
  }
  hit.d = 96;
  hit.mx = mx;
  hit.my = my;
  return hit;
}

bool engine_los (const s_engine *engine, double ax, double ay,
                 double bx, double by)
{
  double d;
  double dx;
  double dy;
  dx = bx - ax;
  dy = by - ay;
  d = hypot(dx, dy);
  if (d < 0.01)
    return true;
  return engine_cast(engine, ax, ay, dx / d, dy / d).d >= d;
}

bool engine_blocked (const s_engine *engine, double x, double y,
                     double r)
{
  return (engine_solid(engine, (int) (x - r), (int) (y - r)) ||
          engine_solid(engine, (int) (x + r), (int) (y - r)) ||
          engine_solid(engine, (int) (x - r), (int) (y + r)) ||
          engine_solid(engine, (int) (x + r), (int) (y + r)));
}

void engine_move (const s_engine *engine, double *x, double *y,
                  double mx, double my, double r)
{
  if (! engine_blocked(engine, *x + mx, *y, r))
    *x += mx;
  if (! engine_blocked(engine, *x, *y + my, r))
    *y += my;
}

void engine_sprites_clear (s_engine *engine)
{
  engine->sprite_count = 0;
}

bool engine_sprite_add (s_engine *engine,
                        const s_engine_sprite *sprite)
{
  if (engine->sprite_count >= ENGINE_SPRITE_MAX)
    return false;
  engine->sprite[engine->sprite_count++] = *sprite;
  return true;
}

void engine_render (s_engine *engine, double px, double py,
                    double pa, uint32_t floor_tex, uint32_t ceil_tex,
                    double fog)
{
  s_engine_hit hit;
  s_engine_sprite_order order[ENGINE_SPRITE_MAX];
  char cell;
  const s_engine_sprite *s;
  const uint32_t *C;
  const uint32_t *F;
  const uint32_t *T;
  double bottom;
  double cx;
  double d;
  double dX;
  double dY;
  double full;
  double fx;
  double fy;
  double h;
  double inv;
  double left;
  double lh;
  double pX;
  double pY;
  double rd;
  double rx;
  double ry;
  double scr;
  double st;
  double sx;
  double sy;
  double tX;
  double tY;
  double top;
  double tp;
  double wx;
  int f;
  int fc;
  int i;
  int n;
  int o;
  int oc;
  int t;
  int tx;
  uint32_t c;
  int x;
  int x0;
  int x1;
  int y;
  int y0;
  int y1;
  dX = cos(pa);
  dY = sin(pa);
  pX = -dY * 0.66;
  pY = dX * 0.66;
  F = engine->tex[floor_tex];
  C = engine->tex[ceil_tex];
  memset(engine->frame, 0, sizeof(engine->frame));
  for (y = ENGINE_HALF + 1; y < ENGINE_VH; y++) {
    rd = (double) ENGINE_HALF / (y - ENGINE_HALF);
    sx = rd * 2 * pX / ENGINE_W;
    sy = rd * 2 * pY / ENGINE_W;
    f = engine_fog(rd, fog);
    fc = (int) (f * 0.8);
    o = y * ENGINE_W;
    oc = (ENGINE_VH - 1 - y) * ENGINE_W;
    fx = px + rd * (dX - pX);
    fy = py + rd * (dY - pY);
    for (x = 0; x < ENGINE_W; x++) {
      t = ((((int) (fy * 64)) & 63) << 6) | (((int) (fx * 64)) & 63);
      fx += sx;
      fy += sy;
      engine->frame[o + x] = engine_shade(F[t], f);
      engine->frame[oc + x] = engine_shade(C[t], fc);
    }
  }
  for (x = 0; x < ENGINE_W; x++) {
    cx = 2.0 * x / ENGINE_W - 1;
    rx = dX + pX * cx;
    ry = dY + pY * cx;
    hit = engine_cast(engine, px, py, rx, ry);
    d = hit.d < 0.05 ? 0.05 : hit.d;
    engine->zbuf[x] = d;
    lh = ENGINE_VH / d;
    top = ENGINE_HALF - lh / 2;
    y0 = (int) top;
    if (y0 < 0)
      y0 = 0;
    y1 = (int) (ENGINE_HALF + lh / 2);
    if (y1 > ENGINE_VH)
      y1 = ENGINE_VH;
    wx = hit.side ? px + d * rx : py + d * ry;
    wx -= floor(wx);
    tx = (int) (wx * 64);
    if (hit.side == 0 && rx < 0)
      tx = 63 - tx;
    if (hit.side == 1 && ry > 0)
      tx = 63 - tx;
    cell = engine_grid_get(engine, hit.mx, hit.my);
    T = engine_wall_tex(engine, cell);
    st = 64.0 / lh;
    f = (int) (engine_fog(d, fog) * (hit.side ? 0.78 : 1.0));
    tp = (y0 - top) * st;
    for (y = y0; y < y1; y++) {
      engine->frame[y * ENGINE_W + x] =
        engine_shade(T[((((int) tp) & 63) << 6) | tx], f);
      tp += st;
    }
  }
  n = engine->sprite_count;
  for (i = 0; i < n; i++) {
    order[i].d = ((engine->sprite[i].x - px) *
                  (engine->sprite[i].x - px) +
                  (engine->sprite[i].y - py) *
                  (engine->sprite[i].y - py));
    order[i].i = i;
  }
  engine_sprite_sort(order, n);
  inv = 1.0 / (pX * dY - dX * pY);
  for (i = 0; i < n; i++) {
    s = &engine->sprite[order[i].i];
    sx = s->x - px;
    sy = s->y - py;
    tX = inv * (dY * sx - dX * sy);
    tY = inv * (-pY * sx + pX * sy);
    if (tY <= 0.1)
      continue;
    scr = (ENGINE_W / 2.0) * (1 + tX / tY);
    full = ENGINE_VH / tY;
    h = full * s->scale;
    bottom = ENGINE_HALF + full / 2 - s->lift * full;
    top = bottom - h;
    left = scr - h / 2;
    x0 = (int) left;
    if (x0 < 0)
      x0 = 0;
    x1 = (int) (scr + h / 2);
    if (x1 > ENGINE_W)
      x1 = ENGINE_W;
    y0 = (int) top;
    if (y0 < 0)
      y0 = 0;
    y1 = (int) bottom;
    if (y1 > ENGINE_VH)
      y1 = ENGINE_VH;
    f = engine_fog(tY, fog);
    T = engine->tex[s->tex];
    for (x = x0; x < x1; x++) {
      if (tY >= engine->zbuf[x])
        continue;
      tx = ((int) ((x - left) * 64 / h)) & 63;
      for (y = y0; y < y1; y++) {
        c = T[(((int) ((y - top) * 64 / h)) & 63) << 6 | tx];
        if ((c >> 24) < 128)
          continue;
        engine->frame[y * ENGINE_W + x] =
          s->flash ? engine_bright(c) : engine_shade(c, f);
      }
    }
  }
}

void engine_blit (s_engine *engine, cairo_t *cr, double w, double h)
{
  cairo_surface_mark_dirty(engine->surface);
  cairo_save(cr);
  cairo_scale(cr, w / ENGINE_W, h / ENGINE_H);
  cairo_set_source_surface(cr, engine->surface, 0.0, 0.0);
  cairo_pattern_set_filter(cairo_get_source(cr),
                           CAIRO_FILTER_NEAREST);
  cairo_paint(cr);
  cairo_restore(cr);
}
