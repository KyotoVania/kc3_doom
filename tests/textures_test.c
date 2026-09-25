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
#include <string.h>
#include <cairo.h>
#include "engine.h"

#define TEST_SCALE 2
#define TEST_COLS  8

extern double textures_rnd_test (uint32_t seed, int n);

static s_engine g_engine;

static int test_atlas (const s_engine *engine);
static int test_pixels (const s_engine *engine);
static int test_rng (void);

int main (void)
{
  if (test_rng())
    return 1;
  if (! textures_init(&g_engine)) {
    fprintf(stderr, "textures_test: textures_init failed\n");
    return 1;
  }
  if (test_pixels(&g_engine))
    return 1;
  if (test_atlas(&g_engine))
    return 1;
  printf("textures_test: OK\n");
  return 0;
}

static int test_atlas (const s_engine *engine)
{
  cairo_t *cr;
  cairo_surface_t *surface;
  cairo_surface_t *tsurf;
  cairo_status_t status;
  unsigned char *tdata;
  int cell;
  int cx;
  int cy;
  int id;
  int rows;
  int stride;
  int tstride;
  int x;
  int y;
  uint32_t a;
  uint32_t b;
  uint32_t g;
  uint32_t px;
  uint32_t r;
  cell = ENGINE_TEX * TEST_SCALE;
  rows = (TEX_COUNT + TEST_COLS - 1) / TEST_COLS;
  surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
                                       TEST_COLS * cell,
                                       rows * cell);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    fprintf(stderr, "textures_test: atlas surface failed\n");
    cairo_surface_destroy(surface);
    return 1;
  }
  cr = cairo_create(surface);
  stride = cell / 16;
  for (y = 0; y < rows * cell / stride; y++)
    for (x = 0; x < TEST_COLS * cell / stride; x++) {
      if ((x + y) % 2)
        cairo_set_source_rgb(cr, 0.42, 0.42, 0.42);
      else
        cairo_set_source_rgb(cr, 0.58, 0.58, 0.58);
      cairo_rectangle(cr, x * stride, y * stride, stride, stride);
      cairo_fill(cr);
    }
  tsurf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                     ENGINE_TEX, ENGINE_TEX);
  tdata = cairo_image_surface_get_data(tsurf);
  tstride = cairo_image_surface_get_stride(tsurf);
  for (id = 0; id < TEX_COUNT; id++) {
    for (y = 0; y < ENGINE_TEX; y++) {
      for (x = 0; x < ENGINE_TEX; x++) {
        px = engine->tex[id][y * ENGINE_TEX + x];
        a = px >> 24;
        r = ((px >> 16) & 0xff) * a / 255;
        g = ((px >> 8) & 0xff) * a / 255;
        b = (px & 0xff) * a / 255;
        px = (a << 24) | (r << 16) | (g << 8) | b;
        memcpy(tdata + y * tstride + x * 4, &px, 4);
      }
    }
    cairo_surface_mark_dirty(tsurf);
    cx = (id % TEST_COLS) * cell;
    cy = (id / TEST_COLS) * cell;
    cairo_save(cr);
    cairo_translate(cr, cx, cy);
    cairo_scale(cr, TEST_SCALE, TEST_SCALE);
    cairo_set_source_surface(cr, tsurf, 0, 0);
    cairo_pattern_set_filter(cairo_get_source(cr),
                             CAIRO_FILTER_NEAREST);
    cairo_paint(cr);
    cairo_restore(cr);
  }
  cairo_surface_destroy(tsurf);
  cairo_destroy(cr);
  status = cairo_surface_write_to_png(surface,
                                      "tests/out/textures_atlas.png");
  cairo_surface_destroy(surface);
  if (status != CAIRO_STATUS_SUCCESS) {
    fprintf(stderr, "textures_test: write_to_png: %s\n",
            cairo_status_to_string(status));
    return 1;
  }
  return 0;
}

static int test_pixels (const s_engine *engine)
{
  int errors;
  int has_opaque;
  int id;
  int i;
  uint32_t a;
  uint32_t b;
  uint32_t g;
  uint32_t px;
  uint32_t r;
  errors = 0;
  for (id = 0; id < TEX_COUNT; id++) {
    has_opaque = 0;
    for (i = 0; i < ENGINE_TEX * ENGINE_TEX; i++) {
      px = engine->tex[id][i];
      a = px >> 24;
      r = (px >> 16) & 0xff;
      g = (px >> 8) & 0xff;
      b = px & 0xff;
      if (a >= 128)
        has_opaque = 1;
      if (a > 0 && (r > 255 || g > 255 || b > 255)) {
        fprintf(stderr, "textures_test: tex %d pixel %d:"
                " impossible channel\n", id, i);
        errors++;
      }
    }
    if (! has_opaque) {
      fprintf(stderr, "textures_test: tex %d: no opaque pixel\n",
              id);
      errors++;
    }
  }
  return errors ? 1 : 0;
}

static int test_rng (void)
{
  static const double ref[5] = {
    0.011704753153, 0.061958257575, 0.976907632779,
    0.699028705712, 0.521445268532
  };
  double v;
  int i;
  for (i = 1; i <= 5; i++) {
    v = textures_rnd_test(7, i);
    if (fabs(v - ref[i - 1]) > 1e-12) {
      fprintf(stderr, "textures_test: rnd #%d: %.12f != %.12f\n",
              i, v, ref[i - 1]);
      return 1;
    }
  }
  v = textures_rnd_test(7, 1001);
  if (fabs(v - 0.555956930853) > 1e-12) {
    fprintf(stderr, "textures_test: rnd #1001: %.12f != %.12f\n",
            v, 0.555956930853);
    return 1;
  }
  return 0;
}
