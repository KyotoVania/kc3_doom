# KMX DOOM, port kc3 + window/cairo (xcb)

Source : `../kmx_doom.html` (550 lignes JS). Les numéros de ligne ci-dessous renvoient à ce fichier.

## Mesures qui fixent la frontière C / kc3

| Mesure (kc3s release, 30 mobs, 60 frames) | ms/frame |
|---|---|
| `update()` en kc3 sans los | 1,7 |
| `update()` en kc3 avec los en cfn C | 1,4 |
| `update()` en kc3 avec los en kc3 | 13,5 |
| séparation mob/mob O(n²) en kc3 | 35 |
| coût d'un appel kc3 (boucle + dispatch) | ~21 µs |

Conséquence : tout ce qui boucle sur des pixels ou des cases de grille va en C, le reste en kc3.
Il faut aussi limiter le nombre de passages entre C et kc3 par frame : pas d'appel cfn par sprite.

## Phase 0 : valider le squelette (fait côté kc3, compilation par toi)

Dépôt autonome : `kmx_doom/`, à côté de `kc3/`.

```
kmx_doom/
  kmx_doom.html   référence JS
  src/kmx_doom.c  hôte C : window_cairo_xcb_run, callbacks, appels kc3 via eval_callable_call
  kc3/doom.kc3    Doom.init/update/key/button/view sur le struct DoomState
  configure       reprend CPPFLAGS/CFLAGS du build kc3, écrit config.mk
  Makefile        make POSIX (fonctionne aussi avec le make d'OpenBSD)
  PLAN.md
```

Contrat C → kc3 :

| Callback C | Appel kc3 | Retour |
|---|---|---|
| `load` | `Doom.init(U32 w, U32 h)` | état |
| `render` | `Doom.update(état, F64 dt)` puis `Doom.view(état)` | `{F64 px, F64 py, F64 dx, F64 dy, Str msg}` |
| `key` | `Doom.key(état, U32 keysym)` | état (Échap intercepté en C) |
| `button` | `Doom.button(état, U8, S64 x, S64 y)` | état |

L'état est une valeur kc3 immuable, que le C garde dans un `s_tag` et remplace à chaque appel.

La séquence kc3 est déjà validée avec `kc3s` (init, key ×4, update, view, button) et la sortie est conforme au contrat.

Build (dépendance à un arbre kc3 déjà configuré et compilé, rien n'est installé) :
1. `./configure` (ou `KC3=/chemin/kc3 ./configure`, par défaut `../kc3`).
   - Il lit `$KC3/window/cairo/xcb/demo/config.mk`, généré par le `configure` de kc3.
   - Il reprend `CPPFLAGS` et `CFLAGS` (donc `-DHAVE_F80=1 -DF80_SIZE=16` et les chemins propres à la plateforme), y ajoute `-I$KC3` et `-DPROG`.
   - Il ajoute `-L` et `-Wl,-rpath` vers `libkc3`, `window`, `window/cairo` et `window/cairo/xcb`, plus `pkg-config --libs cairo`.
   - Vérifié : les flags sont identiques à ceux de la démo xcb.
2. `make`. Si le build kc3 change de flags (reconfigure, passage à OpenBSD), relancer `./configure`.
3. `make run` lance `KC3_DIR=$KC3 KMX_DOOM_KC3=kc3/doom.kc3 ./kmx_doom`.
   - `KC3_DIR` permet à `env_init` de trouver `lib/kc3/0.1` hors de l'arbre kc3 (`libkc3/env.c:2193`).
4. Variante ASan, plus tard : faire la même extraction avec les variables `_ASAN` et `-lkc3_asan`… Les `.so` kc3 doivent correspondre au mode (`make lib_links_linux_asan`).

Ce qu'on doit voir : une grille, un point orange avec sa direction, les flèches qui bougent le point, et la barre du bas avec la touche ou le clic et le compteur de frames.

Points à surveiller au premier lancement (pas encore vérifiés côté C) :
- `env_ident_get` sur `Doom.update` doit renvoyer un `TAG_PCALLABLE`. Sinon, le message d'erreur est explicite.
- Le chemin passé à `kc3_load` est relatif au répertoire courant : `kc3/doom.kc3` par défaut, donc il faut lancer depuis la racine du dépôt.
- Au moment de `kc3_init`, `KC3_DIR` doit être positionné si le binaire n'est pas dans l'arbre kc3.
- `render` est appelé environ 120 fois par seconde par un faux `XCB_EXPOSE` (`window_cairo_xcb.c`, boucle `run`). `dt` est plafonné à 50 ms.
- Le backend xcb inverse y pour `button` (`window->h - y`) mais pas pour `motion`.

## Phase 1 : patches window/ (upstream, à valider avec le mainteneur)

1. **Relâchement de touche** (bloquant pour un déplacement fluide).
   - Ajouter `XCB_EVENT_MASK_KEY_RELEASE` et un `case XCB_KEY_RELEASE` dans `window_cairo_xcb.c`.
   - Ajouter un callback `key_release` avec sa version par défaut dans `window_cairo.c`.
   - Piège : `s_window_cairo` est casté en `s_window` (`window_cairo_init`). Il faut ajouter le champ au même rang dans `s_window` (`window/types.h`) **et** dans tous les backends (sdl2, egl…), ou bien en fin de struct partout.
   - Autorepeat X11 : il génère des paires release/press. On le supprime avec l'autorepeat détectable de xkb, ou on filtre un release suivi immédiatement d'un press de même keycode.
2. **Souris relative** (confort, contournable avec les flèches).
   - `xcb_grab_pointer`, `xcb_warp_pointer` vers le centre, curseur caché, puis delta = position − centre.
   - `conn` et `xcb_window` sont des variables locales de `window_cairo_xcb_run`, inaccessibles à l'hôte. Il faut donc les stocker dans la struct ou ajouter un hook.
3. En attendant : le squelette fonctionne sans ces patches, avec des déplacements par impulsion grâce à l'autorepeat.

## Phase 2 : moteur C (`src/engine.c`)

Le moteur est lié directement à l'exécutable, ou mis dans un `libkmx_doom_engine.so` du dépôt lié au binaire.
- Les `cfn` résolvent via `dlsym(RTLD_DEFAULT)` (`libkc3/cfn.c:354`).
- Dans un `.so` lié, les symboles sont visibles d'office. Dans l'exécutable, il faut `-rdynamic` (`-Wl,--export-dynamic`) dans `LDFLAGS`.
- Dans les deux cas, pas de `dlopen` côté kc3. Il ne faut surtout pas en faire : `dlopen("x.so")` cherche dans `module_path` (`$KC3/lib/kc3/0.1/`), pas dans le dépôt.

- Données : grille `u8[h][w]`, framebuffer `u32[400*200]`, `zbuf[400]`, textures `u32[64*64]` par id, tableau de sprites `{x, y, tex, scale, lift, flash}`.
- `render(cr, px, py, pa)` : porter les lignes 264-300 (sol et plafond, murs DDA, sprites triés avec zbuffer, fog/shade).
- Affichage : `cairo_image_surface_create_for_data(CAIRO_FORMAT_RGB24, 400, 200, 1600)`, puis `cairo_scale` et `CAIRO_FILTER_NEAREST`.
- `cast`, `los`, `blocked`, `move`, `separate` : porter les lignes 248-258, 250 et 375. La version de référence de `los` est prête dans `scratchpad/doom_c.c` : même résultat que la version kc3, 120 vus sur 120.
- Textures (lignes 25-115) : les dessiner avec cairo sur des surfaces 64×64 ARGB32, puis lire les pixels avec `cairo_image_surface_get_data`.
  - Attention : cairo est en ARGB prémultiplié, stocké en BGRA en mémoire, alors que Canvas est en RGBA. Adapter `shade`/`bright` et le test d'alpha `(c>>>24)<128`.
  - Correspondances Canvas → cairo : `fillRect`→`rectangle+fill`, `arc`→`cairo_arc`, `bezierCurveTo`→`cairo_curve_to`, `ellipse`→`save/scale/arc/restore`, `createRadialGradient`→`cairo_pattern_create_radial`, `fillText`→`cairo_show_text` (centrage via `cairo_text_extents`).
  - Le RNG `rnd()` (ligne 24), graine 7, doit être porté à l'identique pour retrouver les mêmes textures.

## Phase 3 : interface kc3 ↔ moteur

- Chargement des fichiers kc3 du dépôt : faire `kc3_load` de chaque fichier depuis `kmx_doom_load`, dans l'ordre des dépendances, ou un `kc3/main.kc3` qui fait des `load("kc3/...")`. `require` cherche seulement dans `$KC3/lib/kc3/0.1`, il ne sert donc que pour la stdlib (`require Time`…).

- Module `DoomEngine` dans `kc3/doom_engine.kc3`, avec des `cfn` seulement, sans `dlopen` :
  - `grid_load(List of Str)` : une fois par niveau ;
  - `grid_set(U32, U32, U8)` : portes ouvertes ;
  - `los(F64 ×4) -> Bool` et `cast(F64 ×4) -> F64` : pour l'IA et `fire()` ;
  - `separate(List) -> List` : un seul appel par frame.
- Rendu : `Doom.view` renvoie tout ce que le C dessine, en un seul tuple : `{px, py, pa, sprites, hud}`, où `sprites` est une List de tuples et `hud` une Map ou un tuple fixe. Le C parcourt la liste, sans aucun appel cfn par sprite.
- Il faut mesurer le coût de construction de `view` en kc3 avec environ 50 sprites. Si c'est trop cher, basculer sur des positions tenues côté C et mises à jour par `DoomEngine.sync(List)`.

## Phase 4 : gameplay en kc3 (lignes 116-379, 482-509)

- Données : `LEVELS` (maps, textes de Thomas, outros), `TY`, `WEAP`, `TIPS`, `ITC`, `MOBC`, sous forme de `def` constants ou de facts.
- `load_level` : parser la map, puis `DoomEngine.grid_load` et les listes de mobs, items et spawns.
- `update` : joueur, IA, projectiles, pickups, messages. Mobs et projectiles en structs, mis à jour par `List.map`.
- `fire`, `hurt_mob`, `hurt_player`, `shoot`, `use`, `take` ; le split de `dfree` ; le boss qui bloque la sortie.
- Machine d'états title / play / pause / dead / inter / win, `new_game`, `restart`, `next_level`.
- Entrées : un ensemble des touches enfoncées dans l'état. Il nécessite `key_release` (phase 1), sinon on reste en mode impulsion.

Pièges kc3 rencontrés pendant les benchs :
- Un struct n'est pas utilisable dans son propre `defmodule` (`struct_type not found`). Il faut un module séparé (`DoomState`, `DoomMob`).
- Il faut un pin quand on déstructure une variable : `{a, b} = ^ t`, `[h | t] = ^ l`, `g = ^ Module.const`.
- L'arithmétique entière redescend au plus petit type : `(U64) (x + 1)` pour un champ U64.
- `sqrt` (`tag_sqrt`) ne gère pas F64 : utiliser `cfn F64 "sqrt" (F64)`.
- `! a && b` se lit `! (a && b)` : toujours mettre des parenthèses.
- Qualifier les appels (`Doom.step`), à cause de la portée dynamique des noms non qualifiés.
- `require Time` si on mesure le temps.

## Phase 5 : HUD, arme, visage, overlays en C/cairo (lignes 381-480)

- kc3 fournit les valeurs dans `view.hud` (hp, ammo, armor, arme, possédées, kills, facts, messages, état, timers). Le C dessine.
- Polices : l'API texte simple de cairo en `monospace` suffit. Sinon, `cairo_font` de `window/cairo`, avec FreeType et `fonts/`.
- Visage du joueur et de Thomas via `?face=` : remplacer par un PNG optionnel (`cairo_image_surface_create_from_png`).

## Phase 6 : son (optionnel)

- Il n'y a pas d'audio dans `window/`. Options : sndio (natif sur OpenBSD, porté sur Linux), ALSA, ou pas de son.
- kc3 met une liste de symboles `:pistol`, `:hurt`… dans `view`, et le C les synthétise (lignes 511-530 : oscillateurs carré/scie, bruit filtré en passe-bas, enveloppes exponentielles).

## Ordre conseillé

0 → 2 (rendu d'une map statique pilotée par le squelette) → 3 → 4 → 5 → 1 → 6.
La phase 1 touche l'upstream : la garder pour la fin, ou la soumettre en parallèle au mainteneur.
