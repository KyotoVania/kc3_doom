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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "libkc3/kc3.h"
#include "window/cairo/window_cairo.h"
#include "window/cairo/xcb/window_cairo_xcb.h"

#define KMX_DOOM_W          400
#define KMX_DOOM_H          250
#define KMX_DOOM_SCALE      2
#define KMX_DOOM_CELL       16.0
#define KMX_DOOM_KEY_ESCAPE 0xff1b
#define KMX_DOOM_MSG_MAX    256

typedef struct kmx_doom {
  s_tag           fn_button;
  s_tag           fn_init;
  s_tag           fn_key;
  s_tag           fn_update;
  s_tag           fn_view;
  s_tag           state;
  struct timespec last;
} s_kmx_doom;

const char *g_env_argv0_default = PROG;
const char *g_env_argv0_dir_default = PREFIX;

static s_kmx_doom g_kmx_doom = {0};

static bool kmx_doom_button (s_window_cairo *window, u8 button,
                             s64 x, s64 y);
static bool kmx_doom_call (s_tag *fn, s_list *arguments,
                           s_tag *dest);
static bool kmx_doom_call_state (s_tag *fn, s_list *arguments);
static bool kmx_doom_fn (const char *name, s_tag *dest);
static bool kmx_doom_key (s_window_cairo *window, u32 keysym);
static bool kmx_doom_load (s_window_cairo *window);
static bool kmx_doom_render (s_window_cairo *window);
static bool kmx_doom_resize (s_window_cairo *window, u64 w, u64 h);
static void kmx_doom_unload (s_window_cairo *window);
static bool kmx_doom_view_f64 (s_tuple *view, uw i, f64 *dest);
static void kmx_doom_view_str (s_tuple *view, uw i, char *dest,
                               uw size);

int main (int argc, char **argv)
{
  s_window_cairo window;
  if (! kc3_init(NULL, &argc, &argv)) {
    err_puts("kc3_init");
    return 1;
  }
  kc3_window_cairo_init();
  window_cairo_init(&window, 0, 0,
                    KMX_DOOM_W * KMX_DOOM_SCALE,
                    KMX_DOOM_H * KMX_DOOM_SCALE,
                    "KMX DOOM", 1);
  window.button = kmx_doom_button;
  window.key    = kmx_doom_key;
  window.load   = kmx_doom_load;
  window.render = kmx_doom_render;
  window.resize = kmx_doom_resize;
  window.unload = kmx_doom_unload;
  if (! window_cairo_xcb_run(&window)) {
    err_puts("window_cairo_xcb_run -> false");
    window_cairo_clean(&window);
    kc3_window_cairo_clean();
    kc3_clean(NULL);
    return g_kc3_exit_code;
  }
  window_cairo_clean(&window);
  kc3_window_cairo_clean();
  kc3_clean(NULL);
  return 0;
}

static bool kmx_doom_button (s_window_cairo *window, u8 button,
                             s64 x, s64 y)
{
  (void) window;
  return kmx_doom_call_state(&g_kmx_doom.fn_button,
                             list_new_u8(button,
                                         list_new_s64(x,
                                                      list_new_s64(y,
                                                                   NULL))));
}

static bool kmx_doom_call (s_tag *fn, s_list *arguments, s_tag *dest)
{
  bool r;
  r = eval_callable_call(fn->data.td_pcallable, arguments, dest);
  list_delete_all(arguments);
  if (! r)
    err_puts("kmx_doom_call: eval_callable_call failed");
  return r;
}

static bool kmx_doom_call_state (s_tag *fn, s_list *arguments)
{
  s_tag tmp = {0};
  if (! kmx_doom_call(fn, list_new_tag_copy(&g_kmx_doom.state,
                                            arguments), &tmp))
    return false;
  tag_clean(&g_kmx_doom.state);
  g_kmx_doom.state = tmp;
  return true;
}

static bool kmx_doom_fn (const char *name, s_tag *dest)
{
  s_ident ident;
  ident_init(&ident, sym_1("Doom"), sym_1(name));
  if (! env_ident_get(env_global(), &ident, dest)) {
    err_write_1("kmx_doom_fn: not found: Doom.");
    err_puts(name);
    return false;
  }
  if (dest->type != TAG_PCALLABLE) {
    err_write_1("kmx_doom_fn: not a Callable: Doom.");
    err_puts(name);
    tag_clean(dest);
    return false;
  }
  return true;
}

static bool kmx_doom_key (s_window_cairo *window, u32 keysym)
{
  (void) window;
  if (keysym == KMX_DOOM_KEY_ESCAPE) {
    g_kc3_exit_code = 0;
    return false;
  }
  return kmx_doom_call_state(&g_kmx_doom.fn_key,
                             list_new_u32(keysym, NULL));
}

static bool kmx_doom_load (s_window_cairo *window)
{
  const char *p;
  s_str path = {0};
  (void) window;
  p = getenv("KMX_DOOM_KC3");
  str_init_1(&path, NULL, p ? p : "kc3/doom.kc3");
  if (! kc3_load(&path)) {
    err_write_1("kmx_doom_load: kc3_load failed: ");
    err_puts(path.ptr.p_pchar);
    return false;
  }
  if (! kmx_doom_fn("button", &g_kmx_doom.fn_button) ||
      ! kmx_doom_fn("init",   &g_kmx_doom.fn_init) ||
      ! kmx_doom_fn("key",    &g_kmx_doom.fn_key) ||
      ! kmx_doom_fn("update", &g_kmx_doom.fn_update) ||
      ! kmx_doom_fn("view",   &g_kmx_doom.fn_view))
    return false;
  if (! kmx_doom_call(&g_kmx_doom.fn_init,
                      list_new_u32(KMX_DOOM_W,
                                   list_new_u32(KMX_DOOM_H, NULL)),
                      &g_kmx_doom.state))
    return false;
  clock_gettime(CLOCK_MONOTONIC, &g_kmx_doom.last);
  return true;
}

static bool kmx_doom_render (s_window_cairo *window)
{
  cairo_t *cr;
  f64 dt;
  f64 dx;
  f64 dy;
  char msg[KMX_DOOM_MSG_MAX];
  struct timespec now;
  f64 px;
  f64 py;
  s_tuple *t;
  s_tag view = {0};
  f64 x;
  cr = window->cr;
  clock_gettime(CLOCK_MONOTONIC, &now);
  dt = (f64) (now.tv_sec - g_kmx_doom.last.tv_sec) +
    (f64) (now.tv_nsec - g_kmx_doom.last.tv_nsec) / 1e9;
  g_kmx_doom.last = now;
  if (dt > 0.05)
    dt = 0.05;
  if (! kmx_doom_call_state(&g_kmx_doom.fn_update,
                            list_new_f64(dt, NULL)))
    return false;
  if (! kmx_doom_call(&g_kmx_doom.fn_view,
                      list_new_tag_copy(&g_kmx_doom.state, NULL),
                      &view))
    return false;
  if (view.type != TAG_PTUPLE ||
      view.data.td_ptuple->count != 5) {
    err_puts("kmx_doom_render: Doom.view must return"
             " {px, py, dx, dy, msg}");
    tag_clean(&view);
    return false;
  }
  t = view.data.td_ptuple;
  if (! kmx_doom_view_f64(t, 0, &px) ||
      ! kmx_doom_view_f64(t, 1, &py) ||
      ! kmx_doom_view_f64(t, 2, &dx) ||
      ! kmx_doom_view_f64(t, 3, &dy)) {
    tag_clean(&view);
    return false;
  }
  kmx_doom_view_str(t, 4, msg, sizeof(msg));
  tag_clean(&view);
  cairo_save(cr);
  cairo_scale(cr, (double) window->w / KMX_DOOM_W,
              (double) window->h / KMX_DOOM_H);
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_paint(cr);
  cairo_set_source_rgb(cr, 0.12, 0.13, 0.15);
  cairo_set_line_width(cr, 1.0);
  for (x = 0.0; x <= KMX_DOOM_W; x += KMX_DOOM_CELL) {
    cairo_move_to(cr, x, 0.0);
    cairo_line_to(cr, x, KMX_DOOM_H);
  }
  for (x = 0.0; x <= KMX_DOOM_H; x += KMX_DOOM_CELL) {
    cairo_move_to(cr, 0.0, x);
    cairo_line_to(cr, KMX_DOOM_W, x);
  }
  cairo_stroke(cr);
  cairo_set_source_rgb(cr, 0.94, 0.63, 0.19);
  cairo_arc(cr, px * KMX_DOOM_CELL, py * KMX_DOOM_CELL, 4.0, 0.0,
            6.283185307179586);
  cairo_fill(cr);
  cairo_set_line_width(cr, 2.0);
  cairo_move_to(cr, px * KMX_DOOM_CELL, py * KMX_DOOM_CELL);
  cairo_line_to(cr, (px + dx) * KMX_DOOM_CELL,
                (py + dy) * KMX_DOOM_CELL);
  cairo_stroke(cr);
  cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL,
                         CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 10.0);
  cairo_set_source_rgb(cr, 1.0, 0.82, 0.23);
  cairo_move_to(cr, 8.0, KMX_DOOM_H - 8.0);
  cairo_show_text(cr, msg);
  cairo_restore(cr);
  return true;
}

static bool kmx_doom_resize (s_window_cairo *window, u64 w, u64 h)
{
  (void) window;
  (void) w;
  (void) h;
  return true;
}

static void kmx_doom_unload (s_window_cairo *window)
{
  (void) window;
  tag_clean(&g_kmx_doom.state);
  tag_clean(&g_kmx_doom.fn_view);
  tag_clean(&g_kmx_doom.fn_update);
  tag_clean(&g_kmx_doom.fn_key);
  tag_clean(&g_kmx_doom.fn_init);
  tag_clean(&g_kmx_doom.fn_button);
  memset(&g_kmx_doom, 0, sizeof(g_kmx_doom));
}

static bool kmx_doom_view_f64 (s_tuple *view, uw i, f64 *dest)
{
  if (view->tag[i].type != TAG_F64) {
    err_write_1("kmx_doom_view_f64: not a F64 at index ");
    err_inspect_uw_decimal(i);
    err_write_1("\n");
    return false;
  }
  *dest = view->tag[i].data.td_f64;
  return true;
}

static void kmx_doom_view_str (s_tuple *view, uw i, char *dest,
                               uw size)
{
  uw len;
  dest[0] = 0;
  if (view->tag[i].type != TAG_STR)
    return;
  len = view->tag[i].data.td_str.size;
  if (len >= size)
    len = size - 1;
  memcpy(dest, view->tag[i].data.td_str.ptr.p_pchar, len);
  dest[len] = 0;
}
