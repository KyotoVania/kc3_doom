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
#include <stdlib.h>
#include "hud.h"

#define HUD_PI 3.141592653589793

static void hud_bar (cairo_t *cr, const s_hud *hud);
static void hud_blink (cairo_t *cr, const s_hud *hud, const char *s,
                       double y);
static void hud_box (cairo_t *cr, double x, double w);
static void hud_concat (char *dest, size_t size, const char *prefix,
                        const char *s);
static void hud_face (cairo_t *cr, const s_hud *hud, double x,
                      double y);
static void hud_flash (cairo_t *cr, double x, double y, double r,
                       double time);
static void hud_fmt (char *dest, size_t size, double t);
static void hud_map (cairo_t *cr, const s_hud *hud,
                     const s_engine *engine);
static void hud_overlay (cairo_t *cr, const s_hud *hud);
static void hud_panel (cairo_t *cr);
static void hud_pct (char *dest, size_t size, int a, int b);
static void hud_play_overlay (cairo_t *cr, const s_hud *hud,
                              const s_engine *engine);
static void hud_rect (cairo_t *cr, uint32_t color, double alpha,
                      double x, double y, double w, double h);
static void hud_set_color (cairo_t *cr, uint32_t color, double alpha);
static void hud_shadow_txt (cairo_t *cr, const char *s, double x,
                            double y, const char *face, double size,
                            bool bold, uint32_t color, double alpha,
                            char align);
static void hud_trap (cairo_t *cr, uint32_t color, double x, double yt,
                      double wt, double yb, double wb);
static void hud_txt (cairo_t *cr, const char *s, double x, double y,
                     const char *face, double size, bool bold,
                     uint32_t color, double alpha, char align);
static void hud_vlabel (cairo_t *cr, const char *s, double x, double y,
                        uint32_t color);
static void hud_weapon (cairo_t *cr, const s_hud *hud);

void hud_draw (cairo_t *cr, const s_hud *hud, const s_engine *engine)
{
  cairo_save(cr);
  if (hud->state != HUD_TITLE) {
    hud_play_overlay(cr, hud, engine);
    hud_bar(cr, hud);
  }
  hud_overlay(cr, hud);
  cairo_restore(cr);
}

static void hud_bar (cairo_t *cr, const s_hud *hud)
{
  char buf[64];
  uint32_t col;
  int i;
  hud_rect(cr, 0x4a4540, 1.0, 0, ENGINE_VH, ENGINE_W,
           ENGINE_H - ENGINE_VH);
  hud_rect(cr, 0x2a2622, 1.0, 0, ENGINE_VH, ENGINE_W, 2);
  hud_box(cr, 0, 68);
  snprintf(buf, sizeof(buf), "%d", hud->ammo);
  hud_txt(cr, buf, 34, ENGINE_VH + 21, "monospace", 20, true,
          0xe0321c, 1.0, 'c');
  hud_txt(cr, "BREAKPOINTS", 34, ENGINE_VH + 40, "monospace", 7, true,
          0xc8bfb2, 1.0, 'c');
  hud_box(cr, 68, 78);
  snprintf(buf, sizeof(buf), "%d%%", hud->hp > 0 ? hud->hp : 0);
  hud_txt(cr, buf, 107, ENGINE_VH + 21, "monospace", 20, true,
          0xe0321c, 1.0, 'c');
  hud_txt(cr, "SANTÉ", 107, ENGINE_VH + 40, "monospace", 7, true,
          0xc8bfb2, 1.0, 'c');
  hud_box(cr, 146, 36);
  for (i = 0; i < 3; i++) {
    buf[0] = '1' + i;
    buf[1] = 0;
    col = hud->owned[i] ? (hud->weapon == i ? 0xffd23a : 0xe8e0d0)
                        : 0x6a625a;
    hud_txt(cr, buf, 154 + i * 10, ENGINE_VH + 15, "monospace", 9,
            true, col, 1.0, 'c');
  }
  hud_txt(cr, hud->weapon_name ? hud->weapon_name : "printf", 164,
          ENGINE_VH + 29, "monospace", 7, true, 0xffd23a, 1.0, 'c');
  hud_txt(cr, "ARMES", 164, ENGINE_VH + 40, "monospace", 7, true,
          0xc8bfb2, 1.0, 'c');
  hud_face(cr, hud, 182, ENGINE_VH + 2);
  hud_box(cr, 218, 78);
  snprintf(buf, sizeof(buf), "%d%%", hud->armor);
  hud_txt(cr, buf, 257, ENGINE_VH + 21, "monospace", 20, true,
          0xe0321c, 1.0, 'c');
  hud_txt(cr, "TESTS", 257, ENGINE_VH + 40, "monospace", 7, true,
          0xc8bfb2, 1.0, 'c');
  hud_box(cr, 296, 104);
  snprintf(buf, sizeof(buf), "BUGS %d/%d", hud->kills, hud->total);
  hud_txt(cr, buf, 348, ENGINE_VH + 14, "monospace", 9, true,
          0xe8e0d0, 1.0, 'c');
  snprintf(buf, sizeof(buf), "FACTS %d/%d", hud->facts, hud->ftotal);
  hud_txt(cr, buf, 348, ENGINE_VH + 27, "monospace", 9, true,
          0x88eeff, 1.0, 'c');
  if (hud->level_name)
    hud_txt(cr, hud->level_name, 348, ENGINE_VH + 40, "monospace", 7,
            true, 0xc8bfb2, 1.0, 'c');
}

static void hud_blink (cairo_t *cr, const s_hud *hud, const char *s,
                       double y)
{
  if (((int) (hud->time * 2)) % 2)
    hud_shadow_txt(cr, s, ENGINE_W / 2, y, "monospace", 10, true,
                   0xffd23a, 1.0, 'c');
}

static void hud_box (cairo_t *cr, double x, double w)
{
  hud_rect(cr, 0x35302b, 1.0, x + 2, ENGINE_VH + 4, w - 4,
           ENGINE_H - ENGINE_VH - 8);
}

static void hud_concat (char *dest, size_t size, const char *prefix,
                        const char *s)
{
  size_t i;
  size_t j;
  i = 0;
  while (i + 1 < size && prefix[i]) {
    dest[i] = prefix[i];
    i++;
  }
  j = 0;
  while (i + 1 < size && s[j]) {
    dest[i] = s[j];
    i++;
    j++;
  }
  dest[i] = 0;
}

static void hud_face (cairo_t *cr, const s_hud *hud, double x, double y)
{
  bool dead;
  int i;
  int lk;
  int n;
  double s;
  dead = hud->hp <= 0;
  hud_rect(cr, 0x1a1714, 1.0, x, y, 36, 46);
  hud_rect(cr, 0xd9a877, 1.0, x + 7, y + 8, 22, 30);
  hud_rect(cr, 0xd9a877, 1.0, x + 10, y + 38, 16, 4);
  hud_rect(cr, 0x4a3020, 1.0, x + 6, y + 4, 24, 7);
  hud_rect(cr, 0x4a3020, 1.0, x + 6, y + 8, 3, 10);
  hud_rect(cr, 0x4a3020, 1.0, x + 27, y + 8, 3, 10);
  s = sin(hud->time * .9);
  lk = 0;
  if (! (hud->hurt_t > 0 || dead))
    lk = s > .7 ? -2 : s < -.7 ? 2 : 0;
  if (dead) {
    hud_txt(cr, "x", x + 13, y + 19, "monospace", 9, true, 0x000000,
            1.0, 'c');
    hud_txt(cr, "x", x + 23, y + 19, "monospace", 9, true, 0x000000,
            1.0, 'c');
  }
  else {
    hud_rect(cr, 0xffffff, 1.0, x + 10, y + 16, 6, 4);
    hud_rect(cr, 0xffffff, 1.0, x + 20, y + 16, 6, 4);
    hud_rect(cr, 0x223344, 1.0, x + 12 + lk, y + 17, 2, 3);
    hud_rect(cr, 0x223344, 1.0, x + 22 + lk, y + 17, 2, 3);
    if (hud->hp < 50) {
      hud_rect(cr, 0x000000, 1.0, x + 10, y + 14, 6, 1);
      hud_rect(cr, 0x000000, 1.0, x + 20, y + 14, 6, 1);
    }
  }
  hud_rect(cr, 0xb88258, 1.0, x + 17, y + 21, 3, 6);
  if (hud->grin_t > 0 && ! dead) {
    hud_rect(cr, 0xffffff, 1.0, x + 11, y + 30, 14, 4);
    hud_rect(cr, 0x000000, 1.0, x + 11, y + 30, 14, 1);
  }
  else if (hud->hurt_t > 0 || dead)
    hud_rect(cr, 0x330000, 1.0, x + 14, y + 29, 8, 6);
  else {
    hud_rect(cr, 0x5a2a1a, 1.0, x + 12, y + 31, 12, 2);
    if (hud->hp < 60) {
      hud_rect(cr, 0x5a2a1a, 1.0, x + 11, y + 32, 2, 2);
      hud_rect(cr, 0x5a2a1a, 1.0, x + 23, y + 32, 2, 2);
    }
  }
  n = (80 - hud->hp) / 10;
  if (n < 0)
    n = 0;
  for (i = 0; i < n; i++)
    hud_rect(cr, 0xaa0000, 1.0, x + 8 + (i * 7) % 20,
             y + 9 + (i * 11) % 26, 2, 3 + i % 3);
}

static void hud_flash (cairo_t *cr, double x, double y, double r,
                       double time)
{
  double a;
  int i;
  double q;
  cairo_new_path(cr);
  for (i = 0; i < 12; i++) {
    a = i / 12.0 * 2 * HUD_PI + time * 20;
    q = i % 2 ? r * .45 : r;
    if (i)
      cairo_line_to(cr, x + cos(a) * q, y + sin(a) * q);
    else
      cairo_move_to(cr, x + cos(a) * q, y + sin(a) * q);
  }
  cairo_close_path(cr);
  hud_set_color(cr, 0xffd24a, 1.0);
  cairo_fill(cr);
  hud_set_color(cr, 0xffffff, 1.0);
  cairo_arc(cr, x, y, r * .35, 0, 2 * HUD_PI);
  cairo_fill(cr);
}

static void hud_fmt (char *dest, size_t size, double t)
{
  int ti;
  ti = (int) t;
  snprintf(dest, size, "%d:%02d", ti / 60, ti % 60);
}

static void hud_map (cairo_t *cr, const s_hud *hud,
                     const s_engine *engine)
{
  char c;
  int cols;
  int i;
  double ox;
  int rows;
  double s;
  int x;
  int y;
  rows = engine->grid_h;
  cols = engine->grid_w;
  if (rows <= 0 || cols <= 0)
    return;
  s = 130.0 / cols;
  if (s > 4)
    s = 4;
  ox = ENGINE_W - cols * s - 4;
  hud_rect(cr, 0x000000, .6, ox - 2, 2, cols * s + 4, rows * s + 4);
  for (y = 0; y < rows; y++)
    for (x = 0; x < cols; x++) {
      c = engine_grid_get(engine, x, y);
      if (c == '.')
        continue;
      hud_rect(cr, c == 'D' ? 0xe8b820 : c == 'X' ? 0x33ff66
                                          : 0x8a8f98,
               1.0, ox + x * s, 4 + y * s, s, s);
    }
  for (i = 0; i < hud->mob_count && i < HUD_MOB_MAX; i++)
    hud_rect(cr, 0xff3333, 1.0, ox + hud->mob_xy[i * 2] * s - 1,
             4 + hud->mob_xy[i * 2 + 1] * s - 1, 2, 2);
  if (hud->has_thomas)
    hud_rect(cr, 0xf0a030, 1.0, ox + hud->thomas_x * s - 1,
             4 + hud->thomas_y * s - 1, 3, 3);
  hud_rect(cr, 0xffffff, 1.0, ox + hud->px * s - 1,
           4 + hud->py * s - 1, 3, 3);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_set_line_width(cr, 1.0);
  cairo_move_to(cr, ox + hud->px * s, 4 + hud->py * s);
  cairo_line_to(cr, ox + (hud->px + cos(hud->pa) * 2) * s,
                4 + (hud->py + sin(hud->pa) * 2) * s);
  cairo_stroke(cr);
}

static void hud_overlay (cairo_t *cr, const s_hud *hud)
{
  char buf[512];
  const char *label;
  char val[64];
  int i;
  const char *ctrl[4];
  switch (hud->state) {
  case HUD_PLAY:
  default:
    break;
  case HUD_TITLE:
    hud_panel(cr);
    hud_txt(cr, "KMX DOOM", ENGINE_W / 2 + 3, 58, "Impact", 46, true,
            0x440000, 1.0, 'c');
    hud_txt(cr, "KMX DOOM", ENGINE_W / 2, 55, "Impact", 46, true,
            0xe0321c, 1.0, 'c');
    hud_txt(cr, "— LE STAGE —", ENGINE_W / 2, 90, "monospace", 13,
            true, 0xffd23a, 1.0, 'c');
    hud_txt(cr, "un FPS en un seul fichier · kmx.io · 2026",
            ENGINE_W / 2, 106, "monospace", 9, false, 0xbbbbbb, 1.0,
            'c');
    ctrl[0] = "ZQSD / WASD : bouger     Souris : viser";
    ctrl[1] = "Clic : tirer     E / Espace : ouvrir, git push";
    ctrl[2] = "1 2 3 : printf, gdb, ASan     Shift : courir";
    ctrl[3] = "M / Tab : carte     Échap : pause";
    for (i = 0; i < 4; i++)
      hud_txt(cr, ctrl[i], ENGINE_W / 2, 138 + i * 13, "monospace", 8,
              false, 0xdddddd, 1.0, 'c');
    hud_blink(cr, hud, "CLIQUE POUR COMMENCER", 215);
    break;
  case HUD_PAUSE:
    hud_panel(cr);
    hud_shadow_txt(cr, "PAUSE", ENGINE_W / 2, 100, "monospace", 30,
                   true, 0xe0321c, 1.0, 'c');
    hud_blink(cr, hud, "clique pour reprendre", 140);
    break;
  case HUD_DEAD:
    hud_rect(cr, 0x780000, .6, 0, 0, ENGINE_W, ENGINE_H);
    hud_shadow_txt(cr, "Segmentation fault", ENGINE_W / 2, 90,
                   "monospace", 22, true, 0xffffff, 1.0, 'c');
    hud_shadow_txt(cr, "(core dumped)", ENGINE_W / 2, 116,
                   "monospace", 12, true, 0xffcccc, 1.0, 'c');
    if (hud->state_t > .8)
      hud_blink(cr, hud, "clique pour relancer le niveau", 160);
    break;
  case HUD_INTER:
    hud_panel(cr);
    hud_shadow_txt(cr, "NIVEAU TERMINÉ", ENGINE_W / 2, 40,
                   "monospace", 22, true, 0xe0321c, 1.0, 'c');
    if (hud->level_name)
      hud_shadow_txt(cr, hud->level_name, ENGINE_W / 2, 64,
                     "monospace", 11, true, 0xffffff, 1.0, 'c');
    for (i = 0; i < 3; i++) {
      if (i == 0) {
        label = "Bugs fixés";
        hud_pct(val, sizeof(val), hud->kills, hud->total);
      }
      else if (i == 1) {
        label = "Facts";
        hud_pct(val, sizeof(val), hud->facts, hud->ftotal);
      }
      else {
        label = "Temps";
        hud_fmt(val, sizeof(val), hud->level_time);
      }
      hud_txt(cr, label, ENGINE_W / 2 - 80, 94 + i * 16, "monospace",
              10, true, 0xdddddd, 1.0, 'l');
      hud_txt(cr, val, ENGINE_W / 2 + 80, 94 + i * 16, "monospace",
              10, true, 0xffd23a, 1.0, 'r');
    }
    hud_concat(buf, sizeof(buf), "Thomas : ",
               hud->outro ? hud->outro : "");
    hud_shadow_txt(cr, buf, ENGINE_W / 2, 160, "monospace", 9, true,
                   0xf0a030, 1.0, 'c');
    if (hud->state_t > .8)
      hud_blink(cr, hud, "clique pour continuer", 200);
    break;
  case HUD_WIN:
    hud_panel(cr);
    hud_txt(cr, "STAGE VALIDÉ", ENGINE_W / 2, 50, "Impact", 30, true,
            0x33ff66, 1.0, 'c');
    hud_concat(buf, sizeof(buf), "Thomas : ",
               hud->outro ? hud->outro
                          : "« Commit validé. Une ligne, sans corps. »");
    hud_shadow_txt(cr, buf, ENGINE_W / 2, 84, "monospace", 9, true,
                   0xf0a030, 1.0, 'c');
    for (i = 0; i < 3; i++) {
      if (i == 0) {
        label = "Bugs fixés";
        snprintf(val, sizeof(val), "%d/%d", hud->g_kills,
                 hud->g_total);
      }
      else if (i == 1) {
        label = "Facts";
        snprintf(val, sizeof(val), "%d/%d", hud->g_facts,
                 hud->g_ftotal);
      }
      else {
        label = "Temps total";
        hud_fmt(val, sizeof(val), hud->g_time);
      }
      hud_txt(cr, label, ENGINE_W / 2 - 80, 112 + i * 15, "monospace",
              10, true, 0xdddddd, 1.0, 'l');
      hud_txt(cr, val, ENGINE_W / 2 + 80, 112 + i * 15, "monospace",
              10, true, 0xffd23a, 1.0, 'r');
    }
    hud_shadow_txt(cr, "Merci Thomas, merci kmx.io.", ENGINE_W / 2,
                   170, "monospace", 10, true, 0xffffff, 1.0, 'c');
    hud_shadow_txt(cr, "Reste plus qu'à écrire le rapport de stage...",
                   ENGINE_W / 2, 186, "monospace", 9, false, 0xbbbbbb,
                   1.0, 'c');
    if (hud->state_t > 1.2)
      hud_blink(cr, hud, "clique pour revenir au titre", 220);
    break;
  }
}

static void hud_panel (cairo_t *cr)
{
  hud_rect(cr, 0x000000, .72, 0, 0, ENGINE_W, ENGINE_H);
}

static void hud_pct (char *dest, size_t size, int a, int b)
{
  if (b)
    snprintf(dest, size, "%d%%", (int) (a * 100.0 / b + .5));
  else
    snprintf(dest, size, "—");
}

static void hud_play_overlay (cairo_t *cr, const s_hud *hud,
                              const s_engine *engine)
{
  double a;
  int i;
  double my;
  hud_weapon(cr, hud);
  hud_rect(cr, 0xffffff, .7, ENGINE_W / 2 - 4, ENGINE_HALF, 3, 1);
  hud_rect(cr, 0xffffff, .7, ENGINE_W / 2 + 2, ENGINE_HALF, 3, 1);
  hud_rect(cr, 0xffffff, .7, ENGINE_W / 2, ENGINE_HALF - 4, 1, 3);
  hud_rect(cr, 0xffffff, .7, ENGINE_W / 2, ENGINE_HALF + 2, 1, 3);
  if (hud->hurt_t > 0)
    hud_rect(cr, 0xff0000, hud->hurt_t * 1.2, 0, 0, ENGINE_W,
             ENGINE_VH);
  if (hud->pick_t > 0)
    hud_rect(cr, 0xffdc50, .25, 0, 0, ENGINE_W, ENGINE_VH);
  my = 6;
  if (hud->thomas_line) {
    hud_rect(cr, 0x0a0c12, .85, 8, 6, ENGINE_W - 16, 34);
    hud_rect(cr, 0xf0a030, 1.0, 8, 6, 3, 34);
    hud_txt(cr, "THOMAS DE GRIVEL · kmx.io", 18, 15, "monospace", 8,
            true, 0xf0a030, 1.0, 'l');
    hud_txt(cr, hud->thomas_line, 18, 29, "monospace", 10, true,
            0xffffff, 1.0, 'l');
    my = 46;
  }
  for (i = 0; i < hud->msg_count && i < HUD_MSG_MAX; i++) {
    a = hud->msg_t[i];
    if (a > 1)
      a = 1;
    if (a < 0)
      a = 0;
    hud_shadow_txt(cr, hud->msg[i], 8, my + 4, "monospace", 8, true,
                   0xffd23a, a, 'l');
    my += 11;
  }
  if (hud->show_map)
    hud_map(cr, hud, engine);
  if (hud->intro > 0) {
    a = hud->intro > 1 ? 1 : hud->intro;
    if (a < 0)
      a = 0;
    if (hud->level_name)
      hud_shadow_txt(cr, hud->level_name, ENGINE_W / 2,
                     ENGINE_HALF - 20, "monospace", 22, true,
                     0xe0321c, a, 'c');
    if (hud->level_sub)
      hud_shadow_txt(cr, hud->level_sub, ENGINE_W / 2, ENGINE_HALF + 4,
                     "monospace", 10, true, 0xffffff, a, 'c');
  }
}

static void hud_rect (cairo_t *cr, uint32_t color, double alpha,
                      double x, double y, double w, double h)
{
  hud_set_color(cr, color, alpha);
  cairo_rectangle(cr, x, y, w, h);
  cairo_fill(cr);
}

static void hud_set_color (cairo_t *cr, uint32_t color, double alpha)
{
  cairo_set_source_rgba(cr, ((color >> 16) & 0xff) / 255.0,
                        ((color >> 8) & 0xff) / 255.0,
                        (color & 0xff) / 255.0, alpha);
}

static void hud_shadow_txt (cairo_t *cr, const char *s, double x,
                            double y, const char *face, double size,
                            bool bold, uint32_t color, double alpha,
                            char align)
{
  hud_txt(cr, s, x + 1, y + 1, face, size, bold, 0x000000, alpha,
          align);
  hud_txt(cr, s, x, y, face, size, bold, color, alpha, align);
}

static void hud_trap (cairo_t *cr, uint32_t color, double x, double yt,
                      double wt, double yb, double wb)
{
  hud_set_color(cr, color, 1.0);
  cairo_new_path(cr);
  cairo_move_to(cr, x - wt / 2, yt);
  cairo_line_to(cr, x + wt / 2, yt);
  cairo_line_to(cr, x + wb / 2, yb);
  cairo_line_to(cr, x - wb / 2, yb);
  cairo_close_path(cr);
  cairo_fill(cr);
}

static void hud_txt (cairo_t *cr, const char *s, double x, double y,
                     const char *face, double size, bool bold,
                     uint32_t color, double alpha, char align)
{
  cairo_text_extents_t ext;
  double tx;
  if (! s)
    return;
  cairo_select_font_face(cr, face, CAIRO_FONT_SLANT_NORMAL,
                         bold ? CAIRO_FONT_WEIGHT_BOLD
                              : CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, size);
  cairo_text_extents(cr, s, &ext);
  tx = x;
  if (align == 'c')
    tx = x - ext.width / 2 - ext.x_bearing;
  else if (align == 'r')
    tx = x - ext.width - ext.x_bearing;
  hud_set_color(cr, color, alpha);
  cairo_move_to(cr, tx, y - (ext.y_bearing + ext.height / 2));
  cairo_show_text(cr, s);
}

static void hud_vlabel (cairo_t *cr, const char *s, double x, double y,
                        uint32_t color)
{
  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_rotate(cr, -HUD_PI / 2);
  hud_txt(cr, s, 0, 0, "monospace", 7, true, color, 1.0, 'c');
  cairo_restore(cr);
}

static void hud_weapon (cairo_t *cr, const s_hud *hud)
{
  double b;
  double bx;
  double by;
  bool fl;
  int i;
  double m;
  double o;
  double rot;
  double x;
  m = hud->moving;
  bx = cos(hud->bob) * 7 * m;
  by = fabs(sin(hud->bob)) * 6 * m + (hud->fire_t > 0 ? 5 : 0);
  x = ENGINE_W / 2 + 16 + bx;
  b = ENGINE_VH + 2 + by;
  fl = hud->fire_t > 0;
  if (hud->weapon == 0) {
    if (fl)
      hud_flash(cr, x, b - 84, 15, hud->time);
    hud_trap(cr, 0x2c2e33, x, b - 80, 14, b - 34, 24);
    hud_trap(cr, 0x484b52, x, b - 80, 5, b - 34, 8);
    hud_vlabel(cr, "printf", x, b - 56, 0xffd23a);
    hud_trap(cr, 0xd9a877, x, b - 44, 30, b, 46);
    hud_trap(cr, 0x1d2a44, x, b - 8, 46, b, 50);
  }
  else if (hud->weapon == 1) {
    if (fl) {
      hud_flash(cr, x - 9, b - 96, 20, hud->time);
      hud_flash(cr, x + 9, b - 96, 20, hud->time);
    }
    hud_trap(cr, 0x3a3d44, x - 9, b - 94, 10, b - 38, 18);
    hud_trap(cr, 0x3a3d44, x + 9, b - 94, 10, b - 38, 18);
    hud_trap(cr, 0x1a1b1f, x, b - 94, 3, b - 38, 4);
    hud_trap(cr, 0x6b3a1a, x, b - 62, 36, b - 36, 46);
    hud_txt(cr, "gdb", x, b - 49, "monospace", 9, true, 0xffd23a,
            1.0, 'c');
    hud_trap(cr, 0xd9a877, x - 22, b - 40, 16, b, 26);
    hud_trap(cr, 0xd9a877, x + 22, b - 40, 16, b, 26);
  }
  else {
    if (fl)
      hud_flash(cr, x, b - 94, 16 + 6.0 * rand() / (RAND_MAX + 1.0),
                hud->time);
    hud_trap(cr, 0x50545c, x, b - 92, 28, b - 30, 50);
    rot = fl ? fmod(hud->time * 40, 6) : 0;
    for (i = 0; i < 4; i++) {
      o = fmod(i * 6 + rot, 24) - 12;
      hud_rect(cr, 0x24262a, 1.0, x + o * .9 - 1, b - 92, 3, 34);
    }
    hud_rect(cr, 0xcc2222, 1.0, x - 22, b - 56, 44, 10);
    hud_txt(cr, "ASan", x, b - 51, "monospace", 8, true, 0xffffff,
            1.0, 'c');
    hud_trap(cr, 0xd9a877, x - 26, b - 36, 16, b, 26);
    hud_trap(cr, 0xd9a877, x + 26, b - 36, 16, b, 26);
  }
}
