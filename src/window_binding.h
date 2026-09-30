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
#ifndef KMX_DOOM_WINDOW_BINDING_H
#define KMX_DOOM_WINDOW_BINDING_H

#include "libkc3/kc3.h"

bool kmx_doom_window_run (s_str *title, u64 w, u64 h, s_tag *callbacks,
                          s_tag *state);

#endif
