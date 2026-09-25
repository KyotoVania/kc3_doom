# REPORT-05 : pont kc3 ↔ moteur C et branchement de l'hôte

## Fichiers

- `src/bridge.h`, `src/bridge.c` : `g_engine`, `g_hud`, les `cfn` (`kmx_doom_los`, `cast`, `blocked`, `grid_load`, `grid_set`, `grid_get`) et `kmx_doom_view_read`.
- `kc3/doom_engine.kc3` : module `DoomEngine`, uniquement des `cfn`, sans `dlopen`.
- `src/kmx_doom.c` : réécrit.
  - Chargement : `engine_init`, puis `textures_init`, puis `kc3_load` de `kc3/data.kc3`, `kc3/doom_engine.kc3` et `kc3/game.kc3` (liste fixe, `KMX_DOOM_KC3` supprimé). Callables résolus : `init`, `update`, `key`, `button`, `motion` et `view`.
  - Rendu d'une frame : `update`, `view`, `kmx_doom_view_read`, `tag_clean(view)`, `engine_render`, fond noir, `engine_blit`, puis `hud_draw` à l'échelle 400×250.
- `configure` : `-Wl,--export-dynamic` en tête de `LDFLAGS`, `-lm` dans `LIBS`.
- `Makefile` : 5 objets, une règle explicite par objet (make POSIX), `make run` sans `KMX_DOOM_KC3`.
- `src/engine.h`, `src/hud.h` : `#include <stdbool.h>` n'est fait que si `LIBKC3_TYPES_H` n'est pas défini (voir « Écarts »).

## Vérifications

- `-fsyntax-only` sur les 5 sources, avec les `CPPFLAGS`/`CFLAGS` exacts du `Makefile` (commandes de `make -n`, `-c … -o …` remplacé par `-fsyntax-only`) : OK, aucun warning.
- Les tests autonomes `engine_test`, `textures_test` et `hud_test`, recompilés après le changement des headers : OK.
- `tests/view_check.kc3` (ajouté après le merge, parce qu'il a besoin de `game.kc3`) : vérifie que `Doom.view` respecte les types attendus par `bridge.c` dans chaque état.
- Non fait : link et `make run`, qui restent chez le propriétaire.

## Écarts avec la fiche

- **`bool`** : libkc3 définit `typedef u8 bool` en C11 (`libkc3/types.h:129`), ce qui entre en conflit avec `<stdbool.h>`. De plus, les types de callback de `window_cairo` renvoient le `bool` de kc3. `engine.h` et `hud.h` n'incluent donc `<stdbool.h>` que hors contexte kc3. Les deux types font 1 octet et valent 0 ou 1, donc les structs partagées gardent le même layout entre les objets compilés avec et sans libkc3. Règle d'inclusion : `libkc3/kc3.h` toujours en premier.
- **Chaînes du HUD** : elles sont copiées dans des buffers statiques de 256 octets et terminées par NUL. Aucun pointeur ne reste dans la vue kc3, qui est nettoyée tout de suite après la lecture. Les Str kc3 ne sont pas garanties terminées par NUL.
- **Clés de la Map `hud`** : recherche linéaire par comparaison de pointeurs de `Sym` (`sym_1`), sans copie (`map_get` copierait la valeur).
- **Champs entiers du HUD** (`hp`, `ammo`…) : tout type numérique est accepté puis tronqué, comme `P.hp|0` dans le JS. Les ids de texture exigent une valeur entière dans `[0, TEX_COUNT)`.
- **Erreur de vue au niveau du tuple** (mauvais `state`, `floor_tex` invalide, clé HUD manquante) : `render` renvoie false, ce qui arrête la fenêtre avec un message explicite. Une erreur dans ce contrat est un bug, et je préfère un arrêt net à 120 messages par seconde. Un sprite invalide, en revanche, est ignoré, avec un seul message par frame.
- **`tests/view_stub.kc3`** : pas écrit, remplacé par `tests/view_check.kc3` sur le vrai `game.kc3`.

## À vérifier au premier `make run`

1. Le link passe avec `--export-dynamic`, et `nm -D kmx_doom | grep kmx_doom_los` montre le symbole exporté.
2. Au chargement, pas de `kmx_doom_fn: not found` : les 6 callables sont résolus.
3. Pas de `cfn_link: dlsym failed` quand `DoomEngine.los` est appelé pour la première fois.
4. L'écran titre s'affiche (vue 3D qui tourne, écran `KMX DOOM`). Un clic ou Entrée lance E1M1.
5. La console n'affiche aucun `kmx_doom_read_*`.
