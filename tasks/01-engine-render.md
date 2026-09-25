# Fiche 01 : moteur de rendu C (raycaster)

## Contexte

`kmx_doom` est le portage en C et kc3 d'un FPS façon Wolfenstein écrit en JS (`kmx_doom.html`).
Le rendu tourne en C pur (sans libkc3) sur un framebuffer 400×200, affiché ensuite par cairo.
Tu implémentes `src/engine.c` selon le contrat fixé dans `src/engine.h`.
**Ne modifie pas `src/engine.h`.** Lis `tasks/README.md`, sections « Règles communes » et « Contrats communs ».

## À lire d'abord

- `src/engine.h` en entier.
- `kmx_doom.html`, lignes 248-300 : `solid`, `blocked`, `move`, `cast`, `los`, `fog`, `shade`, `bright`, `render`.
- `kmx_doom.html`, lignes 20-21 (dimensions) et 150-223 (format des maps).

## À produire

### `src/engine.c`

Implémenter toutes les fonctions de `engine.h`, sauf `textures_init` (fiche 02) :

- `engine_init` : met la struct à zéro et crée `engine->surface` avec `cairo_image_surface_create_for_data((unsigned char *) engine->frame, CAIRO_FORMAT_RGB24, ENGINE_W, ENGINE_VH, ENGINE_W * 4)`. Vérifier que `cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, ENGINE_W) == ENGINE_W * 4`, sinon renvoyer false. Renvoie false si la surface est en erreur.
- `engine_clean` : détruit la surface.
- `engine_grid_load(rows, h)` : copie `h` lignes de même longueur (≤ `ENGINE_GRID_MAX`). Renvoie false si les longueurs diffèrent ou dépassent la limite. Les cellules sont les caractères bruts de la map. Le moteur ne gère que le solide, donc `P`, les mobs, les items et `T` doivent être remplacés par `.` par l'appelant ; en défense, traiter tout caractère hors de `123456DX` comme vide.
- `engine_grid_get`, `engine_grid_set`, `engine_solid` : `engine_solid` renvoie true hors grille ou si la cellule est dans `123456DX`, comme `solid()` ligne 248.
- `engine_cast` : DDA identique à `cast()` lignes 251-257, limite de 96 pas, `d = 96` si rien n'est touché. Si `dx` ou `dy` vaut 0, utiliser `1e30` pour le `delta` correspondant (en JS, `Math.abs(1/0)` donne Infinity).
- `engine_los` : identique à `los()` ligne 258.
- `engine_blocked` et `engine_move` : lignes 249-250. Les casts `(x - r) | 0` du JS sont des troncatures vers zéro : utiliser `(int)`, les coordonnées sont toujours positives dans la grille.
- `engine_sprites_clear` et `engine_sprite_add` : `engine_sprite_add` renvoie false si `ENGINE_SPRITE_MAX` est atteint.
- `engine_render(px, py, pa, floor_tex, ceil_tex, fog)` : port fidèle de `render()` lignes 264-300, sans `ctx.putImageData`.
  - `fog(d) = (int)(max(0.16, 1 - d / fog) * 256)`.
  - Sol et plafond, lignes 268-272. Le plafond utilise `f * 0.8`, tronqué.
  - Murs, lignes 273-280. La texture est donnée par le caractère de la cellule touchée : `1`→`TEX_WALL_1` … `6`→`TEX_WALL_6`, `D`→`TEX_WALL_D`, `X`→`TEX_WALL_X`, et tout autre caractère→`TEX_WALL_6`, comme `WT['6']` ligne 278. Face `side` à 0.78, remplir `zbuf`.
  - Sprites, lignes 286-298. Tri par distance décroissante, projection, test `zbuf`, alpha ≥ 128, `flash` → `bright()`, sinon `shade(c, fog(tY))`.
  - Ne pas porter `Math.random` : la décision de faire clignoter les `race` (ligne 282) revient au code appelant, qui n'ajoute simplement pas le sprite.
- `engine_blit(cr, w, h)` : `cairo_surface_mark_dirty`, puis dessin de la surface mise à l'échelle sur la zone `w × h * (200 / 250)` en haut de la fenêtre. Utiliser `cairo_scale`, `cairo_set_source_surface`, `cairo_pattern_set_filter(..., CAIRO_FILTER_NEAREST)` et `cairo_paint`, entre `cairo_save` et `cairo_restore`.

Pixels : textures en 0xAARRGGBB non prémultiplié, frame en 0x00RRGGBB.
- `shade(c, f)` = chaque canal R, G, B multiplié par `f >> 8`, comme ligne 261.
- `bright(c)` = `((c >> 1) & 0x7f7f7f) + 0x808080`, comme ligne 262.
- Écrire le résultat dans la frame avec l'alpha à 0.

### `tests/engine_test.c`

Programme autonome qui :

1. initialise un `s_engine`, charge la map E1M1 (`kmx_doom.html` lignes 154-172, avec les caractères non muraux remplacés par `.`) ;
2. remplit les textures avec des motifs de test déterministes, sans passer par `textures_init`. Par exemple, pour la texture `id` : `0xFF000000 | (id * 37 << 16) | (x * 4 << 8) | (y * 4)`. Chaque sprite a un disque opaque au centre et des bords en alpha 0 ;
3. vérifie `engine_los` sur au moins 4 cas calculés à la main : même pièce → true, à travers un mur → false, points confondus → true, direction axiale (dx = 0) → pas de NaN ni de boucle infinie ;
4. vérifie `engine_cast` depuis (1.5, 1.5) vers +x : il doit toucher la cellule (5, 1) (`1....1` sur la ligne 1 de la map), à la distance 3.5 ± 1e-9 ;
5. rend une frame depuis la position de `P` (1.5, 1.5) avec pa = 0.3, ajoute 3 sprites, et écrit `tests/out/engine_e1m1.png` via une surface ARGB32 400×250 (frame + fond noir) avec `cairo_surface_write_to_png` ;
6. renvoie 0 et affiche `engine_test: OK` si tout passe, sinon affiche le cas en échec et renvoie 1.

Compiler et lancer exactement ainsi :

```sh
mkdir -p tests/out
cc -std=c11 -W -Wall -Werror -pedantic -O2 $(pkg-config --cflags cairo) \
   -Isrc src/engine.c tests/engine_test.c -o tests/out/engine_test \
   $(pkg-config --libs cairo) -lm
./tests/out/engine_test
```

`textures_init` n'existe pas encore (fiche 02). `engine_test.c` ne doit pas l'appeler, et le link doit fonctionner sans elle.

## Critères d'acceptation

- La compilation ci-dessus passe sans warning, et `engine_test` affiche `engine_test: OK`.
- `tests/out/engine_e1m1.png` montre des murs en perspective, un sol et un plafond texturés, et des sprites masqués derrière les murs. Décris l'image dans le rapport.
- Aucun appel à libkc3, aucune allocation dynamique dans `engine_render`.
- `tests/out/` est ajouté à `.gitignore`.

## Interdits

Ne pas modifier `src/engine.h`, `src/kmx_doom.c` ni `../kc3`. Pas de `make`, pas de commit.

## Livrable

`src/engine.c`, `tests/engine_test.c`, la ligne `tests/out/` dans `.gitignore`, et `tasks/REPORT-01.md`.
