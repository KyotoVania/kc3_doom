#ifndef KMX_DOOM_BRIDGE_H
#define KMX_DOOM_BRIDGE_H

#include "libkc3/kc3.h"
#include "engine.h"
#include "hud.h"

typedef struct kmx_doom_view {
  e_hud_state state;
  f64         px;
  f64         py;
  f64         pa;
  u32         floor_tex;
  u32         ceil_tex;
  f64         fog;
} s_kmx_doom_view;

s_engine * kmx_doom_engine (void);
s_hud *    kmx_doom_hud (void);
bool       kmx_doom_view_read (const s_tag *view, s_kmx_doom_view *dest);

bool       kmx_doom_blocked (f64 x, f64 y, f64 r);
f64        kmx_doom_cast (f64 x, f64 y, f64 dx, f64 dy);
u8         kmx_doom_grid_get (u32 x, u32 y);
bool       kmx_doom_grid_load (p_list *rows);
bool       kmx_doom_grid_set (u32 x, u32 y, u8 c);
bool       kmx_doom_los (f64 ax, f64 ay, f64 bx, f64 by);

#endif /* KMX_DOOM_BRIDGE_H */
