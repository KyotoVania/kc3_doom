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
#ifndef KMX_DOOM_HUD_H
#define KMX_DOOM_HUD_H

#include <stdbool.h>
#include <cairo.h>
#include "engine.h"

#define HUD_MSG_MAX 4
#define HUD_MOB_MAX 256

typedef enum hud_state {
  HUD_TITLE = 0,
  HUD_PLAY,
  HUD_PAUSE,
  HUD_DEAD,
  HUD_INTER,
  HUD_WIN
} e_hud_state;

typedef struct hud {
  e_hud_state state;
  double      state_t;
  double      time;
  double      px;
  double      py;
  double      pa;
  int         hp;
  int         armor;
  int         ammo;
  int         weapon;
  bool        owned[3];
  const char *weapon_name;
  double      fire_t;
  double      hurt_t;
  double      pick_t;
  double      grin_t;
  double      bob;
  double      moving;
  int         kills;
  int         total;
  int         facts;
  int         ftotal;
  double      level_time;
  const char *level_name;
  const char *level_sub;
  const char *outro;
  double      intro;
  const char *msg[HUD_MSG_MAX];
  double      msg_t[HUD_MSG_MAX];
  int         msg_count;
  const char *thomas_line;
  bool        has_thomas;
  double      thomas_x;
  double      thomas_y;
  bool        show_map;
  double      mob_xy[HUD_MOB_MAX * 2];
  int         mob_count;
  int         g_kills;
  int         g_total;
  int         g_facts;
  int         g_ftotal;
  double      g_time;
} s_hud;

void hud_draw (cairo_t *cr, const s_hud *hud, const s_engine *engine);

#endif /* KMX_DOOM_HUD_H */
