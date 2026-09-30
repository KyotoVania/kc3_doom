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
#include "libkc3/kc3.h"
#include <math.h>
#include <string.h>
#include "window/cairo/window_cairo.h"
#include "bridge.h"

#define KMX_DOOM_STR_MAX 256

static bool     g_engine_ready = false;
static s_engine g_engine;
static s_hud    g_hud;
static char     g_hud_weapon_name[KMX_DOOM_STR_MAX];
static char     g_hud_level_name[KMX_DOOM_STR_MAX];
static char     g_hud_level_sub[KMX_DOOM_STR_MAX];
static char     g_hud_outro[KMX_DOOM_STR_MAX];
static char     g_hud_thomas_line[KMX_DOOM_STR_MAX];
static char     g_hud_msg[HUD_MSG_MAX][KMX_DOOM_STR_MAX];
static char     g_grid_rows[ENGINE_GRID_MAX][ENGINE_GRID_MAX + 1];

static const s_tag * kmx_doom_map_find (const s_map *map,
                                        const char *key);
static bool kmx_doom_read_hud (const s_tag *tag);
static bool kmx_doom_read_sprites (const s_tag *tag);
static bool kmx_doom_read_state (const s_tag *tag, e_hud_state *dest);
static bool kmx_doom_tag_bool (const s_tag *tag, bool *dest);
static bool kmx_doom_tag_f64 (const s_tag *tag, f64 *dest);
static bool kmx_doom_tag_int (const s_tag *tag, s64 *dest);
static bool kmx_doom_tag_str (const s_tag *tag, char *dest, uw size);

s_engine * kmx_doom_engine (void)
{
  return &g_engine;
}

s_hud * kmx_doom_hud (void)
{
  return &g_hud;
}

bool kmx_doom_blocked (f64 x, f64 y, f64 r)
{
  return engine_blocked(&g_engine, x, y, r);
}

f64 kmx_doom_cast (f64 x, f64 y, f64 dx, f64 dy)
{
  return engine_cast(&g_engine, x, y, dx, dy).d;
}

u8 kmx_doom_grid_get (u32 x, u32 y)
{
  return (u8) engine_grid_get(&g_engine, (int) x, (int) y);
}

bool kmx_doom_grid_load (p_list *rows)
{
  const char *ptr[ENGINE_GRID_MAX];
  char c;
  int h;
  s_list *l;
  uw i;
  uw len;
  h = 0;
  l = *rows;
  while (l) {
    if (h >= ENGINE_GRID_MAX) {
      err_puts("kmx_doom_grid_load: too many rows");
      return false;
    }
    if (l->tag.type != TAG_STR) {
      err_puts("kmx_doom_grid_load: row is not a Str");
      return false;
    }
    len = l->tag.data.td_str.size;
    if (len > ENGINE_GRID_MAX) {
      err_puts("kmx_doom_grid_load: row too long");
      return false;
    }
    for (i = 0; i < len; i++) {
      c = l->tag.data.td_str.ptr.p_pchar[i];
      if (! strchr("123456DX", c) || ! c)
        c = '.';
      g_grid_rows[h][i] = c;
    }
    g_grid_rows[h][len] = 0;
    ptr[h] = g_grid_rows[h];
    h++;
    l = list_next(l);
  }
  if (! engine_grid_load(&g_engine, ptr, h)) {
    err_puts("kmx_doom_grid_load: engine_grid_load failed");
    return false;
  }
  return true;
}

bool kmx_doom_grid_set (u32 x, u32 y, u8 c)
{
  return engine_grid_set(&g_engine, (int) x, (int) y, (char) c);
}

bool kmx_doom_los (f64 ax, f64 ay, f64 bx, f64 by)
{
  return engine_los(&g_engine, ax, ay, bx, by);
}

bool kmx_doom_view_read (const s_tag *view, s_kmx_doom_view *dest)
{
  s64 i;
  const s_tag *t;
  s_kmx_doom_view tmp = {0};
  if (view->type != TAG_PTUPLE ||
      view->data.td_ptuple->count != 10) {
    err_puts("kmx_doom_view_read: Doom.view must return a 10-tuple"
             " {state, px, py, pa, floor_tex, ceil_tex, fog, sprites,"
             " hud, sfx}");
    return false;
  }
  t = view->data.td_ptuple->tag;
  if (! kmx_doom_read_state(t, &tmp.state))
    return false;
  if (! kmx_doom_tag_f64(t + 1, &tmp.px) ||
      ! kmx_doom_tag_f64(t + 2, &tmp.py) ||
      ! kmx_doom_tag_f64(t + 3, &tmp.pa) ||
      ! kmx_doom_tag_f64(t + 6, &tmp.fog) ||
      ! isfinite(tmp.px) || ! isfinite(tmp.py) || ! isfinite(tmp.pa) ||
      ! isfinite(tmp.fog) || tmp.fog <= 0.0) {
    err_puts("kmx_doom_view_read: invalid px, py, pa or fog");
    return false;
  }
  if (! kmx_doom_tag_int(t + 4, &i) || i < 0 || i >= TEX_COUNT) {
    err_puts("kmx_doom_view_read: invalid floor_tex");
    return false;
  }
  tmp.floor_tex = (u32) i;
  if (! kmx_doom_tag_int(t + 5, &i) || i < 0 || i >= TEX_COUNT) {
    err_puts("kmx_doom_view_read: invalid ceil_tex");
    return false;
  }
  tmp.ceil_tex = (u32) i;
  if (! kmx_doom_read_sprites(t + 7))
    return false;
  if (! kmx_doom_read_hud(t + 8))
    return false;
  if (t[9].type != TAG_PLIST) {
    err_puts("kmx_doom_view_read: sfx is not a List");
    return false;
  }
  g_hud.state = tmp.state;
  g_hud.px = tmp.px;
  g_hud.py = tmp.py;
  g_hud.pa = tmp.pa;
  *dest = tmp;
  return true;
}

static const s_tag * kmx_doom_map_find (const s_map *map,
                                        const char *key)
{
  const s_sym *sym;
  uw i;
  sym = sym_1(key);
  for (i = 0; i < map->count; i++)
    if (map->key[i].type == TAG_PSYM &&
        map->key[i].data.td_psym == sym)
      return map->value + i;
  err_write_1("kmx_doom_map_find: missing hud key: ");
  err_puts(key);
  return NULL;
}

static bool kmx_doom_read_hud (const s_tag *tag)
{
  static const char *f64_keys[] = {
    "state_t", "time", "fire_t", "hurt_t", "pick_t", "grin_t", "bob",
    "moving", "level_time", "intro", "g_time"
  };
  static const char *int_keys[] = {
    "hp", "armor", "ammo", "weapon", "kills", "total", "facts", "ftotal",
    "g_kills", "g_total", "g_facts", "g_ftotal"
  };
  f64 *f64_dest[11];
  f64 f;
  int *int_dest[12];
  s_hud hud = {0};
  uw i;
  s_list *l;
  const s_map *map;
  const s_tag *v;
  const s_tag *vt;
  if (tag->type != TAG_MAP) {
    err_puts("kmx_doom_read_hud: hud is not a Map");
    return false;
  }
  map = &tag->data.td_map;
  f64_dest[0] = &hud.state_t;
  f64_dest[1] = &hud.time;
  f64_dest[2] = &hud.fire_t;
  f64_dest[3] = &hud.hurt_t;
  f64_dest[4] = &hud.pick_t;
  f64_dest[5] = &hud.grin_t;
  f64_dest[6] = &hud.bob;
  f64_dest[7] = &hud.moving;
  f64_dest[8] = &hud.level_time;
  f64_dest[9] = &hud.intro;
  f64_dest[10] = &hud.g_time;
  int_dest[0] = &hud.hp;
  int_dest[1] = &hud.armor;
  int_dest[2] = &hud.ammo;
  int_dest[3] = &hud.weapon;
  int_dest[4] = &hud.kills;
  int_dest[5] = &hud.total;
  int_dest[6] = &hud.facts;
  int_dest[7] = &hud.ftotal;
  int_dest[8] = &hud.g_kills;
  int_dest[9] = &hud.g_total;
  int_dest[10] = &hud.g_facts;
  int_dest[11] = &hud.g_ftotal;
  for (i = 0; i < 11; i++) {
    if (! (v = kmx_doom_map_find(map, f64_keys[i])) ||
        ! kmx_doom_tag_f64(v, f64_dest[i])) {
      err_write_1("kmx_doom_read_hud: invalid ");
      err_puts(f64_keys[i]);
      return false;
    }
  }
  for (i = 0; i < 12; i++) {
    if (! (v = kmx_doom_map_find(map, int_keys[i])) ||
        ! kmx_doom_tag_f64(v, &f) || ! isfinite(f) ||
        f < -2.0e9 || f > 2.0e9) {
      err_write_1("kmx_doom_read_hud: invalid ");
      err_puts(int_keys[i]);
      return false;
    }
    *int_dest[i] = (int) f;
  }
  if (! (v = kmx_doom_map_find(map, "owned")) || v->type != TAG_PLIST) {
    err_puts("kmx_doom_read_hud: owned is not a List");
    return false;
  }
  l = v->data.td_plist;
  for (i = 0; i < 3; i++) {
    if (! l || ! kmx_doom_tag_bool(&l->tag, hud.owned + i)) {
      err_puts("kmx_doom_read_hud: owned must be 3 Bool");
      return false;
    }
    l = list_next(l);
  }
  if (! (v = kmx_doom_map_find(map, "weapon_name")) ||
      ! kmx_doom_tag_str(v, g_hud_weapon_name, KMX_DOOM_STR_MAX) ||
      ! (v = kmx_doom_map_find(map, "level_name")) ||
      ! kmx_doom_tag_str(v, g_hud_level_name, KMX_DOOM_STR_MAX) ||
      ! (v = kmx_doom_map_find(map, "level_sub")) ||
      ! kmx_doom_tag_str(v, g_hud_level_sub, KMX_DOOM_STR_MAX) ||
      ! (v = kmx_doom_map_find(map, "outro")) ||
      ! kmx_doom_tag_str(v, g_hud_outro, KMX_DOOM_STR_MAX)) {
    err_puts("kmx_doom_read_hud: invalid weapon_name, level_name,"
             " level_sub or outro");
    return false;
  }
  hud.weapon_name = g_hud_weapon_name;
  hud.level_name = g_hud_level_name;
  hud.level_sub = g_hud_level_sub;
  hud.outro = g_hud_outro;
  if (! (v = kmx_doom_map_find(map, "thomas_line")))
    return false;
  if (v->type == TAG_VOID)
    hud.thomas_line = NULL;
  else if (kmx_doom_tag_str(v, g_hud_thomas_line, KMX_DOOM_STR_MAX))
    hud.thomas_line = g_hud_thomas_line;
  else {
    err_puts("kmx_doom_read_hud: thomas_line must be Str or void");
    return false;
  }
  if (! (v = kmx_doom_map_find(map, "thomas")))
    return false;
  if (v->type == TAG_VOID)
    hud.has_thomas = false;
  else if (v->type == TAG_PTUPLE && v->data.td_ptuple->count == 2 &&
           kmx_doom_tag_f64(v->data.td_ptuple->tag, &hud.thomas_x) &&
           kmx_doom_tag_f64(v->data.td_ptuple->tag + 1, &hud.thomas_y))
    hud.has_thomas = true;
  else {
    err_puts("kmx_doom_read_hud: thomas must be {x, y} or void");
    return false;
  }
  if (! (v = kmx_doom_map_find(map, "show_map")) ||
      ! kmx_doom_tag_bool(v, &hud.show_map)) {
    err_puts("kmx_doom_read_hud: show_map must be a Bool");
    return false;
  }
  if (! (v = kmx_doom_map_find(map, "msgs")) || v->type != TAG_PLIST) {
    err_puts("kmx_doom_read_hud: msgs is not a List");
    return false;
  }
  l = v->data.td_plist;
  while (l && hud.msg_count < HUD_MSG_MAX) {
    vt = &l->tag;
    if (vt->type != TAG_PTUPLE || vt->data.td_ptuple->count != 2 ||
        ! kmx_doom_tag_str(vt->data.td_ptuple->tag,
                           g_hud_msg[hud.msg_count], KMX_DOOM_STR_MAX) ||
        ! kmx_doom_tag_f64(vt->data.td_ptuple->tag + 1,
                           hud.msg_t + hud.msg_count)) {
      err_puts("kmx_doom_read_hud: msgs must be a List of {Str, F64}");
      return false;
    }
    hud.msg[hud.msg_count] = g_hud_msg[hud.msg_count];
    hud.msg_count++;
    l = list_next(l);
  }
  if (! (v = kmx_doom_map_find(map, "mobs")) || v->type != TAG_PLIST) {
    err_puts("kmx_doom_read_hud: mobs is not a List");
    return false;
  }
  l = v->data.td_plist;
  while (l && hud.mob_count < HUD_MOB_MAX) {
    vt = &l->tag;
    if (vt->type != TAG_PTUPLE || vt->data.td_ptuple->count != 2 ||
        ! kmx_doom_tag_f64(vt->data.td_ptuple->tag,
                           hud.mob_xy + hud.mob_count * 2) ||
        ! kmx_doom_tag_f64(vt->data.td_ptuple->tag + 1,
                           hud.mob_xy + hud.mob_count * 2 + 1)) {
      err_puts("kmx_doom_read_hud: mobs must be a List of {F64, F64}");
      return false;
    }
    hud.mob_count++;
    l = list_next(l);
  }
  g_hud = hud;
  return true;
}

static bool kmx_doom_read_sprites (const s_tag *tag)
{
  bool bad;
  bool flash;
  s64 i;
  s_list *l;
  s_engine_sprite sprite;
  const s_tag *st;
  if (tag->type != TAG_PLIST) {
    err_puts("kmx_doom_read_sprites: sprites is not a List");
    return false;
  }
  engine_sprites_clear(&g_engine);
  bad = false;
  l = tag->data.td_plist;
  while (l) {
    if (l->tag.type != TAG_PTUPLE || l->tag.data.td_ptuple->count != 6) {
      bad = true;
      l = list_next(l);
      continue;
    }
    st = l->tag.data.td_ptuple->tag;
    if (! kmx_doom_tag_f64(st, &sprite.x) ||
        ! kmx_doom_tag_f64(st + 1, &sprite.y) ||
        ! kmx_doom_tag_int(st + 2, &i) ||
        ! kmx_doom_tag_f64(st + 3, &sprite.scale) ||
        ! kmx_doom_tag_f64(st + 4, &sprite.lift) ||
        ! kmx_doom_tag_bool(st + 5, &flash) ||
        i < 0 || i >= TEX_COUNT ||
        ! isfinite(sprite.x) || ! isfinite(sprite.y) ||
        ! isfinite(sprite.scale) || ! isfinite(sprite.lift)) {
      bad = true;
      l = list_next(l);
      continue;
    }
    sprite.tex = (uint32_t) i;
    sprite.flash = flash;
    if (! engine_sprite_add(&g_engine, &sprite))
      break;
    l = list_next(l);
  }
  if (bad)
    err_puts("kmx_doom_read_sprites: invalid sprite(s) ignored, expected"
             " {F64 x, F64 y, tex in [0, TEX_COUNT), F64 scale,"
             " F64 lift, Bool flash}");
  return true;
}

static bool kmx_doom_read_state (const s_tag *tag, e_hud_state *dest)
{
  const s_sym *sym;
  if (tag->type != TAG_PSYM) {
    err_puts("kmx_doom_read_state: state is not a Sym");
    return false;
  }
  sym = tag->data.td_psym;
  if (sym == sym_1("title"))
    *dest = HUD_TITLE;
  else if (sym == sym_1("play"))
    *dest = HUD_PLAY;
  else if (sym == sym_1("pause"))
    *dest = HUD_PAUSE;
  else if (sym == sym_1("dead"))
    *dest = HUD_DEAD;
  else if (sym == sym_1("inter"))
    *dest = HUD_INTER;
  else if (sym == sym_1("win"))
    *dest = HUD_WIN;
  else {
    err_puts("kmx_doom_read_state: unknown state");
    return false;
  }
  return true;
}

static bool kmx_doom_tag_bool (const s_tag *tag, bool *dest)
{
  if (tag->type != TAG_BOOL)
    return false;
  *dest = tag->data.td_bool_ ? true : false;
  return true;
}

static bool kmx_doom_tag_f64 (const s_tag *tag, f64 *dest)
{
  switch (tag->type) {
  case TAG_F32: *dest = tag->data.td_f32; return true;
  case TAG_F64: *dest = tag->data.td_f64; return true;
  case TAG_S8:  *dest = tag->data.td_s8;  return true;
  case TAG_S16: *dest = tag->data.td_s16; return true;
  case TAG_S32: *dest = tag->data.td_s32; return true;
  case TAG_S64: *dest = tag->data.td_s64; return true;
  case TAG_SW:  *dest = tag->data.td_sw;  return true;
  case TAG_U8:  *dest = tag->data.td_u8;  return true;
  case TAG_U16: *dest = tag->data.td_u16; return true;
  case TAG_U32: *dest = tag->data.td_u32; return true;
  case TAG_U64: *dest = tag->data.td_u64; return true;
  case TAG_UW:  *dest = tag->data.td_uw;  return true;
  default: break;
  }
  return false;
}

static bool kmx_doom_tag_int (const s_tag *tag, s64 *dest)
{
  f64 f;
  if (! kmx_doom_tag_f64(tag, &f) || ! isfinite(f) ||
      f != floor(f) || f < -9.0e15 || f > 9.0e15)
    return false;
  *dest = (s64) f;
  return true;
}

static bool kmx_doom_tag_str (const s_tag *tag, char *dest, uw size)
{
  uw len;
  if (tag->type != TAG_STR)
    return false;
  len = tag->data.td_str.size;
  if (len >= size)
    len = size - 1;
  memcpy(dest, tag->data.td_str.ptr.p_pchar, len);
  dest[len] = 0;
  return true;
}

bool kmx_doom_engine_init (void)
{
  if (g_engine_ready)
    return true;
  if (! engine_init(&g_engine)) {
    err_puts("kmx_doom_engine_init: engine_init failed");
    return false;
  }
  if (! textures_init(&g_engine)) {
    err_puts("kmx_doom_engine_init: textures_init failed");
    engine_clean(&g_engine);
    return false;
  }
  g_engine_ready = true;
  return true;
}

void kmx_doom_engine_clean (void)
{
  if (! g_engine_ready)
    return;
  engine_clean(&g_engine);
  g_engine_ready = false;
}

bool kmx_doom_engine_draw (void **window, s_tag *view)
{
  cairo_t *cr;
  s_kmx_doom_view v;
  s_window_cairo *w;
  if (! g_engine_ready) {
    err_puts("kmx_doom_engine_draw: engine not initialized");
    return false;
  }
  if (! window || ! *window || ! view) {
    err_puts("kmx_doom_engine_draw: invalid arguments");
    return false;
  }
  w = *window;
  cr = w->cr;
  if (! cr) {
    err_puts("kmx_doom_engine_draw: no cairo context");
    return false;
  }
  if (! kmx_doom_view_read(view, &v))
    return false;
  engine_render(&g_engine, v.px, v.py, v.pa, v.floor_tex, v.ceil_tex,
                v.fog);
  cairo_save(cr);
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_paint(cr);
  cairo_restore(cr);
  engine_blit(&g_engine, cr, w->w, w->h);
  cairo_save(cr);
  cairo_scale(cr, (double) w->w / ENGINE_W, (double) w->h / ENGINE_H);
  hud_draw(cr, &g_hud, &g_engine);
  cairo_restore(cr);
  return true;
}
