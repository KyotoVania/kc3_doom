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
#include "window/cairo/xcb/window_cairo_xcb.h"
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_BACKEND_TEST_KEY_ESCAPE 0xff1b

static const char *g_window_backend_test_failed = NULL;

static bool window_backend_test_fail (const char *callback)
{
  if (! g_window_backend_test_failed)
    g_window_backend_test_failed = callback;
  fprintf(stderr, "window_backend_test: FAIL %s\n", callback);
  return false;
}

static bool window_backend_test_cairo (cairo_surface_t *surface,
                                       cairo_t *cr,
                                       const char *callback)
{
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS ||
      cairo_status(cr) != CAIRO_STATUS_SUCCESS)
    return window_backend_test_fail(callback);
  return true;
}

bool window_cairo_xcb_run (s_window_cairo *window)
{
  cairo_surface_t *surface;
  if (getenv("WINDOW_BACKEND_TEST_FAIL_LOAD")) {
    fprintf(stderr, "window_backend_test: FAIL_LOAD\n");
    return false;
  }
  if (! window->load)
    return window_backend_test_fail("missing load");
  if (! window->render)
    return window_backend_test_fail("missing render");
  if (! window->button)
    return window_backend_test_fail("missing button");
  if (! window->motion)
    return window_backend_test_fail("missing motion");
  if (! window->resize)
    return window_backend_test_fail("missing resize");
  if (! window->key)
    return window_backend_test_fail("missing key");
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                       (int) window->w,
                                       (int) window->h);
  window->cr = cairo_create(surface);
  if (! window_backend_test_cairo(surface, window->cr, "create"))
    goto clean;
  if (! window->load(window)) {
    window_backend_test_fail("load");
    goto clean;
  }
  if (getenv("WINDOW_BACKEND_TEST_WM_CLOSE")) {
    if (! window->render(window))
      window_backend_test_fail("render");
    else {
      g_kc3_exit_code = 0;
      fprintf(stderr, "window_backend_test: WM_CLOSE\n");
    }
    goto clean;
  }
  if (! window->render(window)) {
    window_backend_test_fail("render");
    goto clean;
  }
  if (! window_backend_test_cairo(surface, window->cr, "render"))
    goto clean;
  if (! window->render(window)) {
    window_backend_test_fail("render");
    goto clean;
  }
  if (! window_backend_test_cairo(surface, window->cr, "render"))
    goto clean;
  if (! window->button(window, (u8) 1, (s64) 10, (s64) 10)) {
    window_backend_test_fail("button");
    goto clean;
  }
  if (! window->motion(window, (s64) 100, (s64) 100)) {
    window_backend_test_fail("motion");
    goto clean;
  }
  if (! window->motion(window, (s64) 140, (s64) 120)) {
    window_backend_test_fail("motion");
    goto clean;
  }
  if (! window->resize(window, window->w * 2, window->h * 2)) {
    window_backend_test_fail("resize");
    goto clean;
  }
  window->w *= 2;
  window->h *= 2;
  cairo_destroy(window->cr);
  cairo_surface_destroy(surface);
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                       (int) window->w,
                                       (int) window->h);
  window->cr = cairo_create(surface);
  if (! window_backend_test_cairo(surface, window->cr, "resize"))
    goto clean;
  if (window->key(window, WINDOW_BACKEND_TEST_KEY_ESCAPE)) {
    window_backend_test_fail("key");
    goto clean;
  }
  g_kc3_exit_code = 0;
  printf("window_backend_test: OK\n");
 clean:
  cairo_destroy(window->cr);
  cairo_surface_destroy(surface);
  window->cr = NULL;
  return false;
}
