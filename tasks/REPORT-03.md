# Rapport fiche 03 : données de jeu en kc3

## Fichiers créés

- `kc3/data.kc3` : modules `DoomTex`, `DoomMobTex`, `DoomMobType`, `DoomTypes`,
  `DoomWeapon`, `DoomWeapons`, `DoomTips`, `DoomItems`, `DoomMobChars`,
  `DoomLevel`, `DoomLevels`. Aucun commentaire, aucun `require`, tous les
  littéraux numériques castés, appels qualifiés.
- `tests/data_test.kc3` : script de vérification (voir plus bas).

## Test

Commande (depuis la racine de la worktree) :

```sh
(cd ../kc3 && . ./env && cd - >/dev/null && ../kc3/kc3s/kc3s --load tests/data_test.kc3 --quit)
```

Sortie :

```
data_test: OK
```

Zéro `FAIL`, aucune erreur `env_eval` ou `buf_parse`.

Le script vérifie : `DoomTex.bproj`/`count`, `DoomTex.floor(:carpet)`,
`DoomMobTex.walk(:race, 1)`/`atk(:boss)`/`dead(:free1)`, le type `:boss`
(hp 900, burst 5, no_pain) et `:leak` (melee), les 3 armes (2e = "gdb",
7 plombs), les 12 astuces (la 1re contient `unquote`), `DoomItems.get("f")`
(tex = `DoomTex.item_fact`, lift 0.25) et `get("x") == void`,
`DoomMobChars.get("B") == :boss`, les 3 niveaux, et pour chaque map :
rectangulaire, exactement un `P`/`T`/`X`, bordure pleine (1re/dernière lignes
et colonnes sans `.`), et un `B` dans E1M3. Les caractères sont comparés par
octet (`Str.char`, codes ASCII 46/80/84/88/66), les maps étant pur ASCII.

## Correspondance DoomTex ↔ enum engine_tex

Vérifiée mécaniquement (script Python comparant `src/engine.h` et
`kc3/data.kc3`) : 48 entrées, toutes OK, `DoomTex.count == (U32) 47`.

| Valeur | enum engine_tex | DoomTex |
|---|---|---|
| 0 | TEX_WALL_1 | `wall_1` |
| 1 | TEX_WALL_2 | `wall_2` |
| 2 | TEX_WALL_3 | `wall_3` |
| 3 | TEX_WALL_4 | `wall_4` |
| 4 | TEX_WALL_5 | `wall_5` |
| 5 | TEX_WALL_6 | `wall_6` |
| 6 | TEX_WALL_D | `wall_d` |
| 7 | TEX_WALL_X | `wall_x` |
| 8 | TEX_FLOOR_TILE | `floor_tile` |
| 9 | TEX_FLOOR_GRATE | `floor_grate` |
| 10 | TEX_FLOOR_CARPET | `floor_carpet` |
| 11 | TEX_FLOOR_PANEL | `floor_panel` |
| 12 | TEX_FLOOR_DARK | `floor_dark` |
| 13 | TEX_SEGV_WALK_0 | `segv_walk_0` |
| 14 | TEX_SEGV_WALK_1 | `segv_walk_1` |
| 15 | TEX_SEGV_ATK | `segv_atk` |
| 16 | TEX_SEGV_DEAD | `segv_dead` |
| 17 | TEX_LEAK_WALK_0 | `leak_walk_0` |
| 18 | TEX_LEAK_WALK_1 | `leak_walk_1` |
| 19 | TEX_LEAK_ATK | `leak_atk` |
| 20 | TEX_LEAK_DEAD | `leak_dead` |
| 21 | TEX_RACE_WALK_0 | `race_walk_0` |
| 22 | TEX_RACE_WALK_1 | `race_walk_1` |
| 23 | TEX_RACE_ATK | `race_atk` |
| 24 | TEX_RACE_DEAD | `race_dead` |
| 25 | TEX_DFREE_WALK_0 | `dfree_walk_0` |
| 26 | TEX_DFREE_WALK_1 | `dfree_walk_1` |
| 27 | TEX_DFREE_ATK | `dfree_atk` |
| 28 | TEX_DFREE_DEAD | `dfree_dead` |
| 29 | TEX_FREE1_WALK_0 | `free1_walk_0` |
| 30 | TEX_FREE1_WALK_1 | `free1_walk_1` |
| 31 | TEX_FREE1_ATK | `free1_atk` |
| 32 | TEX_FREE1_DEAD | `free1_dead` |
| 33 | TEX_BOSS_WALK_0 | `boss_walk_0` |
| 34 | TEX_BOSS_WALK_1 | `boss_walk_1` |
| 35 | TEX_BOSS_ATK | `boss_atk` |
| 36 | TEX_BOSS_DEAD | `boss_dead` |
| 37 | TEX_THOMAS_0 | `thomas_0` |
| 38 | TEX_THOMAS_1 | `thomas_1` |
| 39 | TEX_ITEM_COFFEE | `item_coffee` |
| 40 | TEX_ITEM_AMMO | `item_ammo` |
| 41 | TEX_ITEM_FACT | `item_fact` |
| 42 | TEX_ITEM_GDB | `item_gdb` |
| 43 | TEX_ITEM_ASAN | `item_asan` |
| 44 | TEX_ITEM_ARMOR | `item_armor` |
| 45 | TEX_PROJ | `proj` |
| 46 | TEX_BPROJ | `bproj` |
| 47 | TEX_COUNT | `count` |

## Diff mécanique textes et maps

Script `/tmp/diff_data_03.py` (extraction par regex des chaînes quotées des
deux fichiers, maps, TIPS, thomas, outro, name, sub, msg de TY) :

```sh
python3 /tmp/diff_data_03.py
```

Sortie :

```
map E1M1: identical (19 entries)
map E1M2: identical (20 entries)
map E1M3: identical (23 entries)
TIPS: identical (12 entries)
E1M1 name: identical (1 entries)
E1M1 sub: identical (1 entries)
E1M1 thomas: identical (4 entries)
E1M1 outro: identical (1 entries)
E1M2 name: identical (1 entries)
E1M2 sub: identical (1 entries)
E1M2 thomas: identical (4 entries)
E1M2 outro: identical (1 entries)
E1M3 name: identical (1 entries)
E1M3 sub: identical (1 entries)
E1M3 thomas: identical (4 entries)
E1M3 outro: identical (1 entries)
TY msg texts: identical (9 entries)
```

## Écarts avec le JS

Aucun sur les données. Choix d'implémentation :

- `DoomMobTex.walk(type, frame)` est calculé : `walk_0(type) + (U32) frame`,
  casté en `(U32)` (les frames 0/1 sont adjacentes dans l'enum). Évite le
  piège des littéraux entiers typés U8 dans un pattern.
- `keep` n'est renseigné que pour les tireurs (`:segv` 3.5, `:dfree` 4.0,
  `:boss` 5.0) ; le défaut du struct est `(F64) 0.8`, conforme à la ligne 372
  du JS (`keep` hors de vue vaut .8 et les mêlées utilisent .75 côté logique,
  fiche 06).
- Les valeurs par défaut de `DoomWeapon` (non spécifiées par la fiche) sont
  arbitraires (`name: ""`, `sfx: :none`, etc.) ; les 3 armes de
  `DoomWeapons.all` fixent tous les champs.
- `DoomItems.get` et `DoomMobChars.get` ont une clause attrape-tout qui
  renvoie `void`, comme demandé.

## Points non faits

Aucun.
