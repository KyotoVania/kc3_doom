# Fiche 02 : textures procédurales en cairo

## Contexte

`kmx_doom` porte en C et kc3 un FPS écrit en JS (`kmx_doom.html`). Le JS génère toutes ses textures 64×64 au démarrage, avec l'API Canvas 2D.
Tu portes ces dessins en cairo et tu remplis `engine->tex[id]` dans `textures_init`, déclarée dans `src/engine.h`.
**Ne modifie pas `src/engine.h`.** Lis `tasks/README.md`, sections « Règles communes » et « Contrats communs ».

## À lire d'abord

- `src/engine.h`, en particulier l'`enum engine_tex`.
- `kmx_doom.html`, lignes 23-115 : `rnd`, `tex`, `txt`, `speckle`, `bricks`, `rack`, `oct`, `puddle`, `mob`, `WT`, `FT`, les 6 mobs, `ghost`, `thomasTex` et `IT`.

## À produire

### `src/textures.c`

`bool textures_init (s_engine *engine)` remplit les `TEX_COUNT` textures.

1. **RNG identique au JS.** Porter `rnd()` (ligne 24, mulberry32, graine 7) en `uint32_t` avec la sémantique exacte de `Math.imul`, `>>>` et `|0`. La graine est une variable statique remise à 7 au début de `textures_init`.
   Valeurs de référence avec la graine 7 : les 5 premiers tirages valent `0.011704753153 0.061958257575 0.976907632779 0.699028705712 0.521445268532`, et le 1001e vaut `0.555956930853`.
2. **Même ordre de génération que le JS.** Les textures qui consomment `rnd` doivent être dessinées dans l'ordre du source :
   - `WT` : `1`, `2`, `3`, `4`, `5`, `6`, `D`, `X` ;
   - puis `FT` : `tile`, `grate`, `carpet`, `panel`, `dark` ;
   - puis les mobs dans l'ordre `segv`, `leak`, `race`, `dfree`, `free1`, `boss`, chacun dans l'ordre de `mob()` ligne 32 : `f(0)`, `f(1)`, `f(2)`, `f(3)` ;
   - puis `thomasTex(0)`, `thomasTex(1)` sans visage (branche `else` ligne 102) ;
   - puis `IT`.

   Correspondance avec l'enum :
   - `WT` : `'1'`→`TEX_WALL_1` … `'6'`→`TEX_WALL_6`, `D`→`TEX_WALL_D`, `X`→`TEX_WALL_X`.
   - `FT` : `tile`→`TEX_FLOOR_TILE`, `grate`→`TEX_FLOOR_GRATE`, `carpet`→`TEX_FLOOR_CARPET`, `panel`→`TEX_FLOOR_PANEL`, `dark`→`TEX_FLOOR_DARK`.
   - Mobs : `f(0)`→`WALK_0`, `f(1)`→`WALK_1`, `f(2)`→`ATK`, `f(3)`→`DEAD`.
   - `IT` : `coffee`, `ammo`, `fact`, `gdb`, `asan`, `armor`, `proj`, `bproj` → `TEX_ITEM_*`, `TEX_PROJ`, `TEX_BPROJ`.
3. **Une surface par texture** : `cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 64, 64)`, entièrement transparente au départ, comme un canvas neuf.
4. **Correspondance Canvas → cairo.**
   - `fillStyle` et couleurs CSS → `cairo_set_source_rgba`. Écrire un petit parseur pour `#rgb`, `#rrggbb`, `rgb(r,g,b)` et `rgba(r,g,b,a)`.
   - `fillRect` → `cairo_rectangle` + `cairo_fill`. `strokeRect` → `cairo_rectangle` + `cairo_stroke`, épaisseur courante (1 par défaut).
   - `beginPath` → `cairo_new_path`. `moveTo` et `lineTo` : même comportement, un `line_to` sans point courant agit comme un `move_to`.
   - `arc(x, y, r, a0, a1)` : `a1 = 7` veut dire cercle complet, utiliser `2π`.
   - `ellipse(x, y, rx, ry, 0, 0, 7)` → `cairo_save`, `cairo_translate`, `cairo_scale(rx, ry)`, `cairo_arc(0, 0, 1, 0, 2π)`, `cairo_restore`, puis fill.
   - `bezierCurveTo` → `cairo_curve_to`. `quadraticCurveTo` → `cairo_curve_to`, en convertissant les points de contrôle quadratiques en cubiques.
   - `closePath` → `cairo_close_path`. `fill` et `stroke` consomment le chemin : utiliser `cairo_fill_preserve` si le JS fait `fill()` puis `stroke()` sur le même chemin (ligne 37).
   - `lineWidth` → `cairo_set_line_width`. `strokeStyle` → la source au moment du stroke.
   - `globalAlpha` (dans `speckle`) → multiplier l'alpha de la source.
   - `createRadialGradient` → `cairo_pattern_create_radial` et `cairo_pattern_add_color_stop_rgba`.
   - `txt(g, s, x, y, font, col, al)` : `bold Npx monospace` → `cairo_select_font_face("monospace", NORMAL, BOLD ou NORMAL)` et `cairo_set_font_size(N)`. `textAlign` center, left ou right via `cairo_text_extents`. `textBaseline middle` : y = y − (y_bearing + height / 2).
   - `save` et `restore` → `cairo_save` et `cairo_restore`. `clip` n'est utilisé qu'avec un visage, donc pas de visage à gérer ici.
5. **Copie vers `engine->tex[id]`.** Après `cairo_surface_flush`, lire `cairo_image_surface_get_data`. Le format est ARGB32 prémultiplié, en `uint32_t` natif, sur des lignes de `cairo_image_surface_get_stride` octets.
   Stocker en **non prémultiplié** : pour chaque canal, `c = a ? min(255, c * 255 / a) : 0`, résultat en 0xAARRGGBB.
6. Libérer chaque surface et chaque contexte. Renvoyer false si une surface ou un contexte est en erreur.

La police n'est pas celle d'un navigateur : un rendu visuellement proche suffit. En revanche, positions, couleurs et ordre des tirages `rnd` doivent être fidèles.

### `tests/textures_test.c`

Programme autonome qui :

1. vérifie le RNG. Pour cela, expose une fonction de test non statique `double textures_rnd_test (uint32_t seed, int n)` dans `textures.c`, qui renvoie le n-ième tirage. C'est la seule fonction publique autorisée en plus de `textures_init`. Elle n'apparaît pas dans `engine.h`, le test la déclare lui-même en `extern`. Comparer aux valeurs de référence ci-dessus avec 1e-12 près ;
2. appelle `textures_init` sur un `s_engine` statique. `engine_init` n'est pas nécessaire : `textures_init` n'écrit que dans `tex` ;
3. vérifie que chaque texture a au moins un pixel d'alpha ≥ 128, et qu'aucun pixel n'a un alpha > 0 avec un RGB impossible après dé-prémultiplication (canal > 255) ;
4. écrit `tests/out/textures_atlas.png` : une grille de 8 colonnes, chaque texture agrandie ×2, sur fond damier gris pour voir la transparence ;
5. affiche `textures_test: OK` et renvoie 0.

Compiler et lancer exactement ainsi :

```sh
mkdir -p tests/out
cc -std=c11 -W -Wall -Werror -pedantic -O2 $(pkg-config --cflags cairo) \
   -Isrc src/textures.c tests/textures_test.c -o tests/out/textures_test \
   $(pkg-config --libs cairo) -lm
./tests/out/textures_test
```

## Critères d'acceptation

- La compilation passe sans warning, et `textures_test` affiche `textures_test: OK`.
- Dans l'atlas, on reconnaît :
  - le mur `kmx .io`, le code vert, le blason `OpenBSD`, les racks `OVH`, les briques `SIGSEGV`, la porte jaune/noir `[E]`, le terminal `git push` ;
  - les 5 sols ;
  - les 6 mobs avec leurs 4 poses ;
  - Thomas avec ses 2 poses (bras levé en pose 1) ;
  - les items et les 2 projectiles en dégradé.

  Décris l'atlas dans le rapport, texture par texture, en signalant les écarts.
- `textures.c` ne dépend que de cairo, de libm et de `engine.h`.

## Interdits

Ne pas modifier `src/engine.h`, `src/kmx_doom.c` ni `../kc3`. Pas de `make`, pas de commit. Ne pas implémenter les fonctions `engine_*` : c'est la fiche 01.

## Livrable

`src/textures.c`, `tests/textures_test.c`, la ligne `tests/out/` dans `.gitignore` si elle est absente, et `tasks/REPORT-02.md`.
