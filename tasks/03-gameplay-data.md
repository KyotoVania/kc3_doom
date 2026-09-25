# Fiche 03 : données de jeu en kc3

## Contexte

`kmx_doom` porte en C et kc3 un FPS écrit en JS (`kmx_doom.html`). Toute la logique de jeu est écrite en kc3.
Cette fiche transcrit les **données** du JS en modules kc3 : niveaux, types de mobs, armes, astuces, items et ids de texture.
Aucune logique ici, c'est la fiche 06.
Lis `tasks/README.md` en entier, en particulier les pièges kc3.

## À lire d'abord

- `kmx_doom.html`, lignes 116-224 : `ITC`, `TY`, `MOBC`, `WEAP`, `TIPS`, `LEVELS`.
- `src/engine.h` : l'`enum engine_tex`, qui fixe la valeur numérique de chaque texture.
- `kc3/doom.kc3` : le style kc3 du projet, avec les structs dans des modules séparés.
- Exemples de syntaxe kc3 : `../kc3/lib/kc3/0.1/time.kc3` (defstruct), `../kc3/test/ikc3/struct_update.kc3` (mise à jour de struct), `../kc3/test/ikc3/access.kc3` (accès aux maps).

## À produire

### `kc3/data.kc3`

Un fichier chargé avec `load("kc3/data.kc3")`, sans `require` de fichiers du dépôt. Modules attendus :

1. **`DoomTex`** : une constante U32 par entrée de l'enum, dans le même ordre et avec la même valeur. Par exemple `def wall_1 = (U32) 0`, … `def floor_tile = (U32) 8`, … `def bproj = (U32) 46`.
   Noms : l'enum en minuscules sans le préfixe `TEX_` (`segv_walk_0`, `item_coffee`, `thomas_1`…). Plus `def count = (U32) 47`.
2. **`DoomTex.floor(sym)`** : la texture d'un sol ou d'un plafond, pour `:tile`, `:grate`, `:carpet`, `:panel` et `:dark`.
3. **`DoomMobTex`** : `DoomMobTex.walk(type, frame)`, `DoomMobTex.atk(type)` et `DoomMobTex.dead(type)` pour les types `:segv`, `:leak`, `:race`, `:dfree`, `:free1` et `:boss`. `frame` vaut 0 ou 1.
4. **`DoomMobType`** (module du struct) : `defstruct` avec les champs de `TY` (lignes 118-125).
   - Champs : `hp`, `sp`, `sc`, `ranged`, `melee`, `rate`, `dmg`, `keep`, `flick`, `split`, `burst`, `no_pain`, `msg`.
   - Les nombres sont en F64 ; `ranged`, `melee`, `flick`, `split` et `no_pain` sont des Bool (false par défaut) ; `burst` est un U8 (1 par défaut) ; `msg` est une List de Str.
   - `keep` vaut `(F64) 0.8` par défaut. Voir ligne 372 : `keep` n'est défini que pour les tireurs.
5. **`DoomTypes.get(sym)`** : le `%DoomMobType{}` du type, pour les 6 types.
6. **`DoomWeapon`** (struct) avec `name`, `cd`, `dmin`, `dmax`, `pellets`, `spread`, `ammo` et `sfx` (Sym). **`DoomWeapons.all`** : la List des 3 armes, dans l'ordre de `WEAP` (lignes 128-132).
7. **`DoomTips.all`** : la List des 12 Str de `TIPS` (lignes 134-147), texte identique, accents compris.
8. **`DoomItems.get(str)`** : pour `"c"`, `"a"`, `"f"`, `"g"`, `"z"` et `"v"`, renvoie `{U32 tex, F64 scale, F64 lift}` d'après `ITC` (ligne 116), avec `tex` pris dans `DoomTex`. Renvoie `void` pour un autre caractère.
9. **`DoomMobChars.get(str)`** : pour `"s"`, `"l"`, `"r"`, `"d"` et `"B"`, renvoie le symbole du type d'après `MOBC` (ligne 126). Renvoie `void` sinon.
10. **`DoomLevel`** (struct) avec `name`, `sub`, `floor` (U32 tex), `ceil` (U32 tex), `a` (F64), `fog` (F64), `thomas` (List de Str), `outro` (Str) et `map` (List de Str).
    **`DoomLevels.all`** : la List des 3 niveaux (lignes 149-224), maps recopiées caractère pour caractère. **`DoomLevels.count`** : `(U8) 3`.

Contraintes :
- Aucun commentaire dans le fichier (piège 9 du README).
- Appels et références toujours qualifiés (`DoomTex.wall_1`).
- Tous les littéraux numériques castés.

### `tests/data_test.kc3`

Un script qui fait `load("kc3/data.kc3")` et vérifie, en affichant `FAIL <description>` pour chaque échec :

- `DoomTex.bproj == (U32) 46` et `DoomTex.count == (U32) 47` ;
- `DoomTex.floor(:carpet) == DoomTex.floor_carpet` ;
- `DoomMobTex.walk(:race, 1)`, `DoomMobTex.atk(:boss)` et `DoomMobTex.dead(:free1)` renvoient les bonnes valeurs de l'enum ;
- `DoomTypes.get(:boss)` : `hp` à 900, `burst` à 5, `no_pain` à true. `DoomTypes.get(:leak).melee == true` ;
- 3 armes, la 2e est `"gdb"` avec 7 plombs ;
- 12 astuces, et la 1re contient `unquote` ;
- `DoomItems.get("f")` a un `lift` de 0.25, et `DoomItems.get("x") == void` ;
- `DoomMobChars.get("B") == :boss` ;
- 3 niveaux. Chaque map est rectangulaire (toutes les lignes ont la longueur de la 1re), contient exactement un `P`, un `T` et un `X`, et a une bordure pleine (1re et dernière lignes, 1re et dernière colonnes sans `.`). La map E1M3 contient un `B`.

Si tout passe, le script affiche `data_test: OK` sur la dernière ligne.

Lancer depuis la racine de la worktree :

```sh
(cd ../kc3 && . ./env && cd - >/dev/null && ../kc3/kc3s/kc3s --load tests/data_test.kc3 --quit)
```

## Critères d'acceptation

- La commande ci-dessus se termine par `data_test: OK`, avec zéro `FAIL` et aucune erreur `env_eval` ou `buf_parse`.
- Chaque valeur de `DoomTex` correspond à l'`enum engine_tex` de `src/engine.h`. Joindre la table de correspondance au rapport.
- Textes et maps identiques au JS. Vérifier par un diff mécanique, par exemple extraire les maps des deux fichiers et les comparer, et joindre la commande au rapport.

## Interdits

Ne pas modifier `src/`, `kc3/doom.kc3` ni `../kc3`. Pas de `make`, pas de commit. Ne pas lancer `kc3s` depuis un dossier qui contient un `kc3.dump`.

## Livrable

`kc3/data.kc3`, `tests/data_test.kc3` et `tasks/REPORT-03.md`.
