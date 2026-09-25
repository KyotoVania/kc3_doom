#ifndef KMX_DOOM_ENGINE_H
#define KMX_DOOM_ENGINE_H

#include <stdbool.h>
#include <stdint.h>
#include <cairo.h>

#define ENGINE_W        400
#define ENGINE_H        250
#define ENGINE_VH       200
#define ENGINE_HALF     100
#define ENGINE_TEX      64
#define ENGINE_GRID_MAX 64
#define ENGINE_SPRITE_MAX 512

typedef enum engine_tex {
  TEX_WALL_1 = 0,
  TEX_WALL_2,
  TEX_WALL_3,
  TEX_WALL_4,
  TEX_WALL_5,
  TEX_WALL_6,
  TEX_WALL_D,
  TEX_WALL_X,
  TEX_FLOOR_TILE,
  TEX_FLOOR_GRATE,
  TEX_FLOOR_CARPET,
  TEX_FLOOR_PANEL,
  TEX_FLOOR_DARK,
  TEX_SEGV_WALK_0,
  TEX_SEGV_WALK_1,
  TEX_SEGV_ATK,
  TEX_SEGV_DEAD,
  TEX_LEAK_WALK_0,
  TEX_LEAK_WALK_1,
  TEX_LEAK_ATK,
  TEX_LEAK_DEAD,
  TEX_RACE_WALK_0,
  TEX_RACE_WALK_1,
  TEX_RACE_ATK,
  TEX_RACE_DEAD,
  TEX_DFREE_WALK_0,
  TEX_DFREE_WALK_1,
  TEX_DFREE_ATK,
  TEX_DFREE_DEAD,
  TEX_FREE1_WALK_0,
  TEX_FREE1_WALK_1,
  TEX_FREE1_ATK,
  TEX_FREE1_DEAD,
  TEX_BOSS_WALK_0,
  TEX_BOSS_WALK_1,
  TEX_BOSS_ATK,
  TEX_BOSS_DEAD,
  TEX_THOMAS_0,
  TEX_THOMAS_1,
  TEX_ITEM_COFFEE,
  TEX_ITEM_AMMO,
  TEX_ITEM_FACT,
  TEX_ITEM_GDB,
  TEX_ITEM_ASAN,
  TEX_ITEM_ARMOR,
  TEX_PROJ,
  TEX_BPROJ,
  TEX_COUNT
} e_engine_tex;

typedef struct engine_sprite {
  double   x;
  double   y;
  uint32_t tex;
  double   scale;
  double   lift;
  bool     flash;
} s_engine_sprite;

typedef struct engine_hit {
  double d;
  int    mx;
  int    my;
  int    side;
} s_engine_hit;

typedef struct engine {
  int              grid_w;
  int              grid_h;
  char             grid[ENGINE_GRID_MAX][ENGINE_GRID_MAX];
  uint32_t         tex[TEX_COUNT][ENGINE_TEX * ENGINE_TEX];
  uint32_t         frame[ENGINE_W * ENGINE_VH];
  double           zbuf[ENGINE_W];
  s_engine_sprite  sprite[ENGINE_SPRITE_MAX];
  int              sprite_count;
  cairo_surface_t *surface;
} s_engine;

bool         engine_init (s_engine *engine);
void         engine_clean (s_engine *engine);

bool         engine_grid_load (s_engine *engine, const char **rows,
                               int h);
bool         engine_grid_set (s_engine *engine, int x, int y, char c);
char         engine_grid_get (const s_engine *engine, int x, int y);
bool         engine_solid (const s_engine *engine, int x, int y);

s_engine_hit engine_cast (const s_engine *engine, double x, double y,
                          double dx, double dy);
bool         engine_los (const s_engine *engine, double ax, double ay,
                         double bx, double by);
bool         engine_blocked (const s_engine *engine, double x, double y,
                             double r);
void         engine_move (const s_engine *engine, double *x, double *y,
                          double mx, double my, double r);

void         engine_sprites_clear (s_engine *engine);
bool         engine_sprite_add (s_engine *engine,
                                const s_engine_sprite *sprite);

void         engine_render (s_engine *engine, double px, double py,
                            double pa, uint32_t floor_tex,
                            uint32_t ceil_tex, double fog);
void         engine_blit (s_engine *engine, cairo_t *cr, double w,
                          double h);

bool         textures_init (s_engine *engine);

#endif /* KMX_DOOM_ENGINE_H */
