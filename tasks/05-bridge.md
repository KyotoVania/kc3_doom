# Fiche 05 : pont kc3 ↔ moteur C, et branchement de l'hôte

**Prérequis** : les fiches 01 (`src/engine.c`), 02 (`src/textures.c`), 03 (`kc3/data.kc3`) et 04 (`src/hud.c`) sont mergées.
La fiche 06 peut tourner en parallèle, car elle utilise un stub à la place de `DoomEngine`.

## Contexte

`kmx_doom` porte en C et kc3 un FPS écrit en JS (`kmx_doom.html`).
L'hôte `src/kmx_doom.c` ouvre une fenêtre `window_cairo_xcb` (bibliothèque window de kc3) et appelle chaque frame des fonctions kc3 via `eval_callable_call`.
Tu écris :
- le pont dans les deux sens : fonctions C appelables depuis kc3 (`cfn`), et lecture en C de la vue renvoyée par kc3 ;
- le branchement de l'hôte sur le moteur (`engine_render`, `engine_blit`) et sur le HUD (`hud_draw`).

Lis `tasks/README.md` en entier, en particulier « Contrats communs » et « Vue par frame ».

## À lire d'abord

- `src/kmx_doom.c` : l'hôte actuel. Il appelle déjà `Doom.init/update/key/button/view` ; la vue actuelle, `{px, py, dx, dy, msg}`, est provisoire et doit être remplacée.
- `src/engine.h`, `src/hud.h`, `kc3/data.kc3`, `configure`, `Makefile`, `PLAN.md` (phases 2 et 3).
- API C de kc3, en lecture seule, dans `../kc3/libkc3/` :
  - `types.h` : `s_tag` et `u_tag_data`. Champs utiles : `td_f64`, `td_u32`, `td_u8`, `td_bool_` (attention au `_` final), `td_str` (`.size`, `.ptr.p_pchar`), `td_psym`, `td_ptuple` (`->count`, `->tag[i]`), `td_plist`, `td_map` (`.count`, `.key[i]`, `.value[i]`). Types correspondants : `TAG_F64`, `TAG_U32`, `TAG_BOOL`, `TAG_STR`, `TAG_PSYM`, `TAG_PTUPLE`, `TAG_PLIST`, `TAG_MAP`, `TAG_VOID`.
  - `list.h` : `list_next`.
  - `map.h` : `map_get (const s_map *, const s_tag *key, s_tag *dest)`.
  - `sym.h` : `sym_1`.
  - `cfn.c` ligne 354 : la résolution des `cfn` par `dlsym(RTLD_DEFAULT, …)`.
  - Mapping des types `cfn` : `F64`→`f64`, `U32`→`u32`, `U8`→`u8`, `Bool`→`bool`. Un argument `List` est passé comme `p_list *` (`s_list **`), comme dans `plist_each (p_list *plist, …)` de `plist.h`. Vérifier sur `plist.h` avant d'écrire la signature.
- Exemples de `cfn` : `../kc3/lib/kc3/0.1/list.kc3`, `../kc3/lib/kc3/0.1/event_poll.kc3`.

## À produire

### 1. `src/bridge.h` et `src/bridge.c` (C avec libkc3)

- Un `static s_engine g_engine` et un `static s_hud g_hud`. Accesseurs : `s_engine * kmx_doom_engine (void)` et `s_hud * kmx_doom_hud (void)`.
- Fonctions exportées, appelées par kc3 via `cfn`. Noms préfixés `kmx_doom_`, non `static` :
  - `bool kmx_doom_grid_load (p_list *rows)` : List de Str, les lignes de la map. Remplacer par `.` tout caractère hors de `123456DX` avant d'appeler `engine_grid_load`.
  - `bool kmx_doom_grid_set (u32 x, u32 y, u8 c)`.
  - `u8 kmx_doom_grid_get (u32 x, u32 y)`.
  - `bool kmx_doom_los (f64 ax, f64 ay, f64 bx, f64 by)`.
  - `f64 kmx_doom_cast (f64 x, f64 y, f64 dx, f64 dy)` : renvoie `engine_cast(...).d`.
  - `bool kmx_doom_blocked (f64 x, f64 y, f64 r)`.
- `bool kmx_doom_view_read (const s_tag *view, s_kmx_doom_view *dest)`. La struct contient `state`, `px`, `py`, `pa`, `floor_tex`, `ceil_tex` et `fog`. La fonction :
  - remplit `g_engine` avec les sprites (`engine_sprites_clear` puis `engine_sprite_add`) ;
  - remplit `g_hud` à partir de la Map `hud` ;
  - joue ou ignore la liste `sfx`. Pas de son pour l'instant : lire et ignorer, mais valider les types.

  Chaque erreur de type ou de clé manquante écrit un message explicite avec `err_puts` et renvoie false. Les pointeurs `const char *` de `g_hud` pointent dans les Str de `view`, donc l'appelant garde `view` vivant jusqu'après `hud_draw`, puis fait `tag_clean`. Sur ce point, l'utilisation dans `kmx_doom.c` doit respecter la durée de vie.

**Contrat de la Map `hud`**, clé Sym → type. C'est ce que la fiche 06 doit produire :

| clé | type | champ `s_hud` |
|---|---|---|
| `state_t` `time` `fire_t` `hurt_t` `pick_t` `grin_t` `bob` `moving` `level_time` `intro` `g_time` | F64 | même nom |
| `hp` `armor` `ammo` `weapon` `kills` `total` `facts` `ftotal` `g_kills` `g_total` `g_facts` `g_ftotal` | U32 | même nom (int) |
| `owned` | List de 3 Bool | `owned[3]` |
| `weapon_name` `level_name` `level_sub` `outro` | Str | même nom |
| `msgs` | List de `{Str, F64}` (4 au plus) | `msg[]`, `msg_t[]`, `msg_count` |
| `thomas_line` | Str ou `void` | `thomas_line` (NULL si void) |
| `thomas` | `{F64 x, F64 y}` ou `void` | `has_thomas`, `thomas_x`, `thomas_y` |
| `show_map` | Bool | `show_map` |
| `mobs` | List de `{F64 x, F64 y}` | `mob_xy`, `mob_count` (tronquer à `HUD_MOB_MAX`) |

Lecture numérique tolérante : kc3 redescend l'arithmétique entière au plus petit type (piège 3 du README), et certaines valeurs peuvent arriver en F64. Pour tout champ numérique du tableau, et pour `floor_tex`, `ceil_tex` et `tex`, accepter `TAG_U8`, `TAG_U16`, `TAG_U32`, `TAG_U64`, `TAG_S8`, `TAG_S16`, `TAG_S32`, `TAG_S64`, `TAG_F32` et `TAG_F64`, puis convertir. Écrire un seul helper `kmx_doom_tag_f64` et un seul `kmx_doom_tag_int`.

`state_sym` (1er élément du tuple) donne `hud.state` : `:title`→`HUD_TITLE`, `:play`, `:pause`, `:dead`, `:inter`, `:win`. `px`, `py` et `pa` du tuple sont copiés dans `g_hud`.

### 2. `kc3/doom_engine.kc3`

Module `DoomEngine` qui ne contient que des `cfn` vers les fonctions ci-dessus, **sans `dlopen`** : les symboles sont dans l'exécutable, exportés par `--export-dynamic`.

```
def los = cfn Bool "kmx_doom_los" (F64, F64, F64, F64)
```

Et de même pour `grid_load (List)`, `grid_set (U32, U32, U8)`, `grid_get`, `cast` et `blocked`.

### 3. `src/kmx_doom.c`

- `load` :
  - `engine_init` et `textures_init` ;
  - `kc3_load` de `kc3/data.kc3`, `kc3/doom_engine.kc3` puis `kc3/game.kc3` (fiche 06), avant `kc3/doom.kc3` s'il existe encore. Remplacer `KMX_DOOM_KC3` par une liste fixe de fichiers relatifs à la racine du dépôt ;
  - résoudre les callables : `Doom.init`, `Doom.update`, `Doom.key`, `Doom.button`, `Doom.motion` et `Doom.view`. Ajouter `motion`.
- `render` : `Doom.update(state, dt)`, puis `Doom.view(state)`, puis `kmx_doom_view_read`. Si `state == :title`, garder quand même le rendu 3D (le JS rend toujours la vue, ligne 541). Ensuite, `engine_render`, `engine_blit(cr, window->w, window->h)`, puis `cairo_save`, `cairo_scale(w / 400, h / 250)`, `hud_draw`, `cairo_restore`, et enfin `tag_clean(&view)`.
- `key` : Échap reste intercepté en C.
- `unload` : `engine_clean` en plus.

### 4. `configure` et `Makefile`

- `LDFLAGS` : ajouter `-Wl,--export-dynamic`. Sans lui, `dlsym(RTLD_DEFAULT)` ne voit pas les symboles de l'exécutable.
- Ajouter `-lm`.
- `OBJECTS` : ajouter `src/engine.o`, `src/textures.o`, `src/hud.o` et `src/bridge.o`, avec une règle par objet sur le modèle de `src/kmx_doom.o`. Pas de règle générique GNU (`%.o`) : le `Makefile` doit rester POSIX pour le make d'OpenBSD.

### 5. `tests/view_stub.kc3`

Un module `Doom` minimal de test, qui renvoie une vue conforme au contrat à partir de `kc3/data.kc3` : niveau E1M1, 3 sprites, HUD complet.
Lancé avec `kc3s`, il affiche `inspect(Doom.view(Doom.init((U32) 400, (U32) 250)))` et `view_stub: OK`. Ce stub sert au propriétaire pour tester l'hôte sans la fiche 06.

## Vérification autorisée

Tu ne peux pas linker l'hôte, car c'est le propriétaire qui compile. Deux vérifications sont autorisées :

```sh
./configure
cc $(sed -n 's/^CPPFLAGS = //p' config.mk) $(sed -n 's/^CFLAGS = //p' config.mk) \
   -Isrc -fsyntax-only src/bridge.c src/kmx_doom.c
```

et le script kc3 :

```sh
(cd ../kc3 && . ./env && cd - >/dev/null && ../kc3/kc3s/kc3s --load tests/view_stub.kc3 --quit)
```

Si le `sed` perd des guillemets dans `CPPFLAGS`, le signaler dans le rapport au lieu de modifier `configure` pour contourner le problème.

## Critères d'acceptation

- `-fsyntax-only` passe sans warning sur `bridge.c` et `kmx_doom.c`.
- `view_stub.kc3` affiche `view_stub: OK`.
- Aucun appel `cfn` par sprite. Par frame, il y a exactement 2 appels kc3 depuis C (`update`, `view`), plus un par événement d'entrée.
- Le rapport liste ce que le propriétaire doit vérifier au premier `make run` : callables trouvés, types de la vue, durée de vie des Str.

## Interdits

Ne pas modifier `src/engine.h`, `src/hud.h` ni `../kc3`. Pas de `make`, pas de link, pas de commit.

## Livrable

`src/bridge.h`, `src/bridge.c`, `kc3/doom_engine.kc3`, `tests/view_stub.kc3`, les modifications de `src/kmx_doom.c`, `configure` et `Makefile`, et `tasks/REPORT-05.md`.
