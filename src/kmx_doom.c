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
#include <string.h>
#include "bridge.h"
#include "window_binding.h"

const char *g_env_argv0_default = PROG;

const char *g_env_argv0_dir_default = PREFIX;

static const char *g_kmx_doom_files[] = {
  "kc3/data.kc3",
  "kc3/doom_engine.kc3",
  "kc3/game.kc3",
  "kc3/doom_window.kc3",
  "kc3/doom_app_state.kc3",
  "kc3/doom_app.kc3",
  NULL
};

static bool kmx_doom_load_file (const char *path);
static bool kmx_doom_main (void);

int main (int argc, char **argv)
{
  int i;
  int r = 1;
  if (! kc3_init(NULL, &argc, &argv)) {
    err_puts("kc3_init");
    return 1;
  }
  kc3_window_cairo_init();
  for (i = 0; g_kmx_doom_files[i]; i++)
    if (! kmx_doom_load_file(g_kmx_doom_files[i]))
      goto clean;
  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--load") || i + 1 == argc) {
      err_puts("usage: kmx_doom [--load file]");
      goto clean;
    }
    if (! kmx_doom_load_file(argv[++i]))
      goto clean;
  }
  if (kmx_doom_main())
    r = 0;
  else if (g_kc3_exit_code != 0)
    err_puts("DoomApp.main -> false");
 clean:
  kmx_doom_engine_clean();
  kc3_window_cairo_clean();
  kc3_clean(NULL);
  return r;
}

static bool kmx_doom_load_file (const char *path)
{
  s_str str = {0};
  str_init_1(&str, NULL, path);
  if (! kc3_load(&str)) {
    err_write_1("kmx_doom_load_file: kc3_load failed: ");
    err_puts(path);
    return false;
  }
  return true;
}

static bool kmx_doom_main (void)
{
  s_ident ident;
  s_tag fn = {0};
  s_tag result = {0};
  bool r;
  ident_init(&ident, sym_1("DoomApp"), sym_1("main"));
  if (! env_ident_get(env_global(), &ident, &fn)) {
    err_puts("kmx_doom_main: DoomApp.main not found");
    return false;
  }
  if (fn.type != TAG_PCALLABLE) {
    err_puts("kmx_doom_main: DoomApp.main is not a Callable");
    tag_clean(&fn);
    return false;
  }
  if (! eval_callable_call(fn.data.td_pcallable, NULL, &result)) {
    err_puts("kmx_doom_main: DoomApp.main failed");
    tag_clean(&fn);
    return false;
  }
  r = result.type == TAG_BOOL && result.data.td_bool_;
  tag_clean(&result);
  tag_clean(&fn);
  return r;
}
