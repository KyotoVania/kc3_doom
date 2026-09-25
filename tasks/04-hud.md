# Fiche 04 : HUD, arme, visage et écrans en cairo

## Contexte

`kmx_doom` porte en C et kc3 un FPS écrit en JS (`kmx_doom.html`). La logique est en kc3, qui transmet chaque frame à C l'état à afficher.
Tu portes tout le dessin 2D par-dessus la vue 3D : l'arme, le réticule, les flashs, la bulle de Thomas, les messages, la minimap, la barre de HUD, le visage, et les écrans titre, pause, mort, fin de niveau et victoire.
Le dessin prend en entrée une struct C simple, `s_hud`, que tu définis exactement comme ci-dessous. Le remplissage depuis kc3 est l'affaire de la fiche 05.
Lis `tasks/README.md`, sections « Règles communes » et « Contrats communs ».

## À lire d'abord

- `kmx_doom.html`, lignes 381-480 : `trap`, `flash`, `vlabel`, `drawWeapon`, `drawFace`, `drawHUD`, `drawMap`, `shadowTxt`, `drawPlayOverlay`, `panel`, `blink`, `fmt`, `pct`, `drawOverlay`.
- `kmx_doom.html`, ligne 26 : `txt` ; ligne 541-543 : l'ordre d'appel dans `frame`.
- `src/engine.h` : pour la minimap, le HUD lit la grille via `engine_grid_get`, déclarée dans `engine.h` et implémentée par la fiche 01.

## Contrat `s_hud` (dans `src/hud.h`, exactement)

```c
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
```

Sémantique :
- `thomas_line` vaut NULL quand Thomas est absent, trop loin ou hors de vue. Sinon, c'est la ligne déjà choisie par kc3 (ligne 446 du JS).
- `mob_xy` contient les positions des mobs vivants, sous la forme x0, y0, x1, y1…
- `weapon_name` est le nom de l'arme courante.

## À produire

### `src/hud.h` (le contrat ci-dessus) et `src/hud.c`

`hud_draw` dessine dans le repère logique 400×250 du JS. L'appelant a déjà appliqué `cairo_scale` pour la fenêtre, et la vue 3D est déjà dessinée dans les 200 premières lignes.
Reproduire l'ordre de la ligne 542 : si `state != HUD_TITLE`, `drawPlayOverlay` puis `drawHUD` ; puis `drawOverlay` dans tous les cas.

Correspondance des primitives :
- `txt` et `shadowTxt` : mêmes règles que dans la fiche 02 (centrage par `cairo_text_extents`, baseline `middle`).
- `Impact, monospace` : essayer `cairo_select_font_face("Impact", …)`, qui retombe tout seul sur une police par défaut. Ne pas embarquer de police.
- `trap` : chemin à 4 points puis fill. `flash` : l'étoile à 12 branches de la ligne 382.
- `vlabel` : `save`, `translate`, `rotate(-π/2)`, `txt`, `restore`.
- `globalAlpha` : `cairo_push_group` puis `cairo_pop_group_to_source` et `cairo_paint_with_alpha`, ou alpha multiplié dans chaque source. Garder un rendu identique.
- `drawFace` : uniquement la branche sans image `FACE` (lignes 406-416).
- `drawMap` : lire la grille avec `engine_grid_get(engine, x, y)` sur `engine->grid_w` × `engine->grid_h`. Couleurs : `D` jaune, `X` vert, autre mur gris, `.` rien.
- `Math.random` (ligne 396, flash de l'ASan) : utiliser `rand()` de la libc, c'est seulement cosmétique.

Textes : reprendre à l'identique ceux du JS (accents UTF-8, `«»`, `—`). Pour `fmt` et `pct`, voir lignes 457-458.

### `tests/hud_test.c`

Programme autonome qui dessine `hud_draw` sur un fond gris 400×250 et écrit un PNG pour chacun de ces cas :

1. `hud_title.png` : `HUD_TITLE`, `time` = 0.6 pour que le texte « CLIQUE POUR COMMENCER » soit visible.
2. `hud_play_printf.png` : `HUD_PLAY`, arme 0, `fire_t` > 0, 2 messages, `thomas_line` renseignée, `show_map` à true et 5 mobs. Il faut une grille 24×19 chargée, avec au moins une porte, pour la minimap.
3. `hud_play_gdb_hurt.png` : arme 1, `hurt_t` = 0.2, hp 40, `grin_t` = 0.
4. `hud_play_asan_intro.png` : arme 2, `intro` = 2, `level_name` et `level_sub` renseignés.
5. `hud_dead.png` : `HUD_DEAD`, `state_t` = 1.
6. `hud_inter.png` : `HUD_INTER`, avec des stats.
7. `hud_win.png` : `HUD_WIN`, avec des stats globales et `state_t` = 2.

La grille de test se remplit directement dans une struct `s_engine` statique : poser `grid_w`, `grid_h` et les caractères de `grid`.
Le test ne linke pas `engine.c`. Il fournit donc sa propre définition de `engine_grid_get`, marquée comme un stub de test en tête du fichier de test, dans le nom de la fonction ou d'une variable et pas en commentaire.

Compiler et lancer exactement ainsi :

```sh
mkdir -p tests/out
cc -std=c11 -W -Wall -Werror -pedantic -O2 $(pkg-config --cflags cairo) \
   -Isrc src/hud.c tests/hud_test.c -o tests/out/hud_test \
   $(pkg-config --libs cairo) -lm
./tests/out/hud_test
```

Le programme affiche `hud_test: OK` après avoir écrit les 7 PNG dans `tests/out/`.

## Critères d'acceptation

- La compilation passe sans warning, `hud_test: OK` s'affiche et les 7 PNG existent.
- Décrire chaque PNG dans le rapport, en comparant avec le JS : positions, couleurs, textes.
- `hud.c` ne dépend que de cairo, de libm, de la libc, de `engine.h` et de `hud.h`.

## Interdits

Ne pas modifier `src/engine.h`, `src/kmx_doom.c` ni `../kc3`. Ne pas implémenter les fonctions `engine_*` dans `src/` : c'est la fiche 01. Pas de `make`, pas de commit.

## Livrable

`src/hud.h`, `src/hud.c`, `tests/hud_test.c`, la ligne `tests/out/` dans `.gitignore` si elle est absente, et `tasks/REPORT-04.md`.
