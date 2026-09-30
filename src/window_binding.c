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
#include "window/cairo/window_cairo.h"
#include "window/cairo/xcb/window_cairo_xcb.h"
#include <string.h>
#include <time.h>
#include "window_binding.h"

#define KMX_DOOM_WINDOW_TITLE_MAX 255
#define KMX_DOOM_WINDOW_SIZE_MAX  16384
#define KMX_DOOM_WINDOW_EXIT_SENTINEL 1

typedef enum kmx_doom_cb {
  CB_BUTTON,
  CB_KEY,
  CB_LOAD,
  CB_MOTION,
  CB_RENDER,
  CB_RESIZE,
  CB_UNLOAD,
  CB_COUNT
} e_kmx_doom_cb;

typedef struct kmx_doom_window {
  s_window_cairo  window;
  s_tag           fn[CB_COUNT];
  s_tag           state;
  struct timespec last;
  char            title[KMX_DOOM_WINDOW_TITLE_MAX + 1];
  bool            cleaned;
  bool            failed;
  bool            loaded;
  bool            stopped;
} s_kmx_doom_window;

static const char *g_kmx_doom_cb_names[CB_COUNT] = {
  "button", "key", "load", "motion", "render", "resize", "unload"
};

static bool kmx_doom_window_button (s_window_cairo *window, u8 button,
                                    s64 x, s64 y);
static bool kmx_doom_window_call (s_kmx_doom_window *ctx,
                                  e_kmx_doom_cb cb, s_list *extra);
static void kmx_doom_window_cleanup (s_kmx_doom_window *ctx);
static bool kmx_doom_window_fns (s_kmx_doom_window *ctx,
                                 const s_tag *callbacks);
static bool kmx_doom_window_key (s_window_cairo *window, u32 keysym);
static bool kmx_doom_window_load (s_window_cairo *window);
static bool kmx_doom_window_motion (s_window_cairo *window, s64 x,
                                    s64 y);
static bool kmx_doom_window_render (s_window_cairo *window);
static bool kmx_doom_window_resize (s_window_cairo *window, u64 w,
                                    u64 h);
static bool kmx_doom_window_result (s_kmx_doom_window *ctx,
                                    s_tag *result, bool *go);
static void kmx_doom_window_unload (s_window_cairo *window);

bool kmx_doom_window_run (s_str *title, u64 w, u64 h, s_tag *callbacks,
                          s_tag *state)
{
  s_kmx_doom_window ctx;
  bool r;
  uw n;
  if (! title || ! callbacks || ! state) {
    err_puts("kmx_doom_window_run: invalid arguments");
    return false;
  }
  if (w == 0 || h == 0 || w > KMX_DOOM_WINDOW_SIZE_MAX ||
      h > KMX_DOOM_WINDOW_SIZE_MAX) {
    err_puts("kmx_doom_window_run: invalid window size");
    return false;
  }
  if (callbacks->type != TAG_MAP) {
    err_puts("kmx_doom_window_run: callbacks must be a Map");
    return false;
  }
  memset(&ctx, 0, sizeof(ctx));
  n = title->size;
  if (n > KMX_DOOM_WINDOW_TITLE_MAX)
    n = KMX_DOOM_WINDOW_TITLE_MAX;
  memcpy(ctx.title, title->ptr.p_pchar, n);
  if (! kmx_doom_window_fns(&ctx, callbacks))
    return false;
  if (! tag_init_copy(&ctx.state, state)) {
    kmx_doom_window_cleanup(&ctx);
    return false;
  }
  if (! window_cairo_init(&ctx.window, 0, 0, w, h, ctx.title, 1)) {
    kmx_doom_window_cleanup(&ctx);
    return false;
  }
  ctx.window.button = kmx_doom_window_button;
  ctx.window.key    = kmx_doom_window_key;
  ctx.window.load   = kmx_doom_window_load;
  ctx.window.motion = kmx_doom_window_motion;
  ctx.window.render = kmx_doom_window_render;
  ctx.window.resize = kmx_doom_window_resize;
  ctx.window.unload = kmx_doom_window_unload;
  g_kc3_exit_code = KMX_DOOM_WINDOW_EXIT_SENTINEL;
  r = window_cairo_xcb_run(&ctx.window);
  ctx.window.cr = NULL;
  kmx_doom_window_cleanup(&ctx);
  window_cairo_clean(&ctx.window);
  if (ctx.failed)
    return false;
  if (ctx.stopped) {
    g_kc3_exit_code = 0;
    return true;
  }
  return r || g_kc3_exit_code == 0;
}

static bool kmx_doom_window_button (s_window_cairo *window, u8 button,
                                    s64 x, s64 y)
{
  return kmx_doom_window_call((s_kmx_doom_window *) window, CB_BUTTON,
                              list_new_u8(button,
                                          list_new_s64(x,
                                                       list_new_s64(y,
                                                                    NULL))));
}

static bool kmx_doom_window_call (s_kmx_doom_window *ctx,
                                  e_kmx_doom_cb cb, s_list *extra)
{
  s_list *args;
  bool go = false;
  s_tag result = {0};
  bool r;
  if (ctx->failed || ctx->stopped) {
    list_delete_all(extra);
    return false;
  }
  args = list_new_tag_copy(&ctx->state,
                           list_new_ptr(ctx, extra));
  if (! args) {
    list_delete_all(extra);
    ctx->failed = true;
    return false;
  }
  r = eval_callable_call(ctx->fn[cb].data.td_pcallable, args,
                         &result);
  list_delete_all(args);
  if (! r) {
    tag_clean(&result);
    err_write_1("kmx_doom_window_call: callback failed: ");
    err_puts(g_kmx_doom_cb_names[cb]);
    ctx->failed = true;
    return false;
  }
  if (! kmx_doom_window_result(ctx, &result, &go)) {
    err_write_1("kmx_doom_window_call: error or invalid result: ");
    err_puts(g_kmx_doom_cb_names[cb]);
    ctx->failed = true;
    return false;
  }
  if (! go) {
    ctx->stopped = true;
    return false;
  }
  return true;
}

static void kmx_doom_window_cleanup (s_kmx_doom_window *ctx)
{
  uw i;
  if (ctx->cleaned)
    return;
  ctx->cleaned = true;
  if (ctx->loaded && ctx->fn[CB_UNLOAD].type == TAG_PCALLABLE) {
    s_list *args;
    s_tag result = {0};
    bool go;
    args = list_new_tag_copy(&ctx->state, list_new_ptr(ctx, NULL));
    if (args) {
      if (! eval_callable_call(ctx->fn[CB_UNLOAD].data.td_pcallable,
                               args, &result)) {
        err_puts("kmx_doom_window_cleanup: unload callback failed");
        tag_clean(&result);
        ctx->failed = true;
      }
      else if (! kmx_doom_window_result(ctx, &result, &go)) {
        err_puts("kmx_doom_window_cleanup: invalid unload result");
        ctx->failed = true;
      }
      list_delete_all(args);
    }
    else
      ctx->failed = true;
  }
  for (i = 0; i < CB_COUNT; i++)
    tag_clean(ctx->fn + i);
  tag_clean(&ctx->state);
}

static bool kmx_doom_window_fns (s_kmx_doom_window *ctx,
                                 const s_tag *callbacks)
{
  uw i;
  s_tag key = {0};
  for (i = 0; i < CB_COUNT; i++) {
    tag_init_psym(&key, sym_1(g_kmx_doom_cb_names[i]));
    if (! map_get(&callbacks->data.td_map, &key, ctx->fn + i)) {
      err_write_1("kmx_doom_window_run: missing callback: ");
      err_puts(g_kmx_doom_cb_names[i]);
      goto ko;
    }
    if (ctx->fn[i].type != TAG_PCALLABLE) {
      err_write_1("kmx_doom_window_run: not a Callable: ");
      err_puts(g_kmx_doom_cb_names[i]);
      goto ko;
    }
  }
  return true;
 ko:
  for (i = 0; i < CB_COUNT; i++)
    tag_clean(ctx->fn + i);
  memset(ctx->fn, 0, sizeof(ctx->fn));
  return false;
}

static bool kmx_doom_window_key (s_window_cairo *window, u32 keysym)
{
  return kmx_doom_window_call((s_kmx_doom_window *) window, CB_KEY,
                              list_new_u32(keysym, NULL));
}

static bool kmx_doom_window_load (s_window_cairo *window)
{
  s_kmx_doom_window *ctx = (s_kmx_doom_window *) window;
  clock_gettime(CLOCK_MONOTONIC, &ctx->last);
  ctx->loaded = true;
  return kmx_doom_window_call(ctx, CB_LOAD, NULL);
}

static bool kmx_doom_window_motion (s_window_cairo *window, s64 x,
                                    s64 y)
{
  return kmx_doom_window_call((s_kmx_doom_window *) window, CB_MOTION,
                              list_new_s64(x, list_new_s64(y, NULL)));
}

static bool kmx_doom_window_render (s_window_cairo *window)
{
  s_kmx_doom_window *ctx = (s_kmx_doom_window *) window;
  f64 dt;
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  dt = (f64) (now.tv_sec - ctx->last.tv_sec) +
    (f64) (now.tv_nsec - ctx->last.tv_nsec) / 1e9;
  ctx->last = now;
  return kmx_doom_window_call(ctx, CB_RENDER, list_new_f64(dt, NULL));
}

static bool kmx_doom_window_resize (s_window_cairo *window, u64 w,
                                    u64 h)
{
  return kmx_doom_window_call((s_kmx_doom_window *) window, CB_RESIZE,
                              list_new_u64(w, list_new_u64(h, NULL)));
}

static bool kmx_doom_window_result (s_kmx_doom_window *ctx,
                                    s_tag *result, bool *go)
{
  s_tag next = {0};
  if (result->type != TAG_PTUPLE ||
      result->data.td_ptuple->count != 2 ||
      result->data.td_ptuple->tag[0].type != TAG_BOOL) {
    tag_clean(result);
    return false;
  }
  if (! tag_init_copy(&next, result->data.td_ptuple->tag + 1)) {
    tag_clean(result);
    return false;
  }
  *go = result->data.td_ptuple->tag[0].data.td_bool_ ? true : false;
  tag_clean(result);
  tag_clean(&ctx->state);
  ctx->state = next;
  return true;
}

static void kmx_doom_window_unload (s_window_cairo *window)
{
  kmx_doom_window_cleanup((s_kmx_doom_window *) window);
}
