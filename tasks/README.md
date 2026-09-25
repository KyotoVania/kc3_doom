# Fiches de tâches kmx_doom (flotte Kimi)

Chaque fiche `NN-*.md` est autonome : un agent qui ne connaît pas l'historique doit pouvoir l'exécuter seul.
Référence fonctionnelle : `kmx_doom.html` (le jeu JS d'origine). Les numéros de ligne cités renvoient à ce fichier.
Vue d'ensemble : `PLAN.md`.

## Ordre et parallélisme

| Fiche | Langage | Dépend de | Parallélisable avec |
|---|---|---|---|
| 01-engine-render | C (pur, cairo) | `src/engine.h` | 02, 03, 04 |
| 02-textures | C (pur, cairo) | `src/engine.h` | 01, 03, 04 |
| 03-gameplay-data | kc3 | rien | 01, 02, 04 |
| 04-hud | C (pur, cairo) | `src/engine.h` | 01, 02, 03 |
| 05-bridge | C (libkc3) + kc3 | 01, 03 mergés | 06 (avec stub) |
| 06-gameplay-logic | kc3 | 03 mergé | 05 |

Vague 1 : 01, 02, 03 et 04 en parallèle.
Vague 2 : 05 et 06, une fois la vague 1 relue et mergée.
Intégration finale (brancher `src/kmx_doom.c` sur le moteur, le HUD et le jeu) : à la main ou dans une fiche 07 écrite après la vague 2.

## Lancer une fiche

Depuis la racine de `kmx_doom/`, une worktree par fiche pour éviter les conflits :

```sh
mkdir -p logs
git worktree add ../kmx_doom-01 -b task/01
cd ../kmx_doom-01
kimi -y --add-dir ../kc3 -p "$(cat tasks/01-engine-render.md)" \
     --output-format stream-json > ../kmx_doom/logs/01.jsonl
```

- `-y` exécute les éditions courantes et demande confirmation pour les actions risquées. Éviter `--auto` tant que le comportement n'est pas connu.
- `--add-dir ../kc3` donne accès aux sources kc3. Depuis une worktree `../kmx_doom-NN`, `../kc3` pointe toujours vers `kc3git/kc3`.
- En vague 1, on peut lancer les 4 fiches dans 4 terminaux.

Revue puis merge :

```sh
cd ../kmx_doom
git diff master...task/01
git merge task/01
git worktree remove ../kmx_doom-01
git branch -d task/01
```

## Règles communes à toutes les fiches (rappelées dans chacune)

- Ne jamais compiler ni modifier kc3 (`../kc3`), qui est en lecture seule. Ne pas lancer `make` dans `kmx_doom`.
- Autorisé : compiler et exécuter les tests **autonomes** de `tests/` (C pur + cairo, sans libkc3) avec la commande exacte donnée par la fiche. Exécuter `kc3s` sur des scripts de `tests/` avec la commande donnée.
- Ne pas commiter : le propriétaire relit et commite lui-même.
- Aucun commentaire dans le code, sauf l'en-tête de licence quand un fichier voisin en a un.
- Style C : celui de `src/kmx_doom.c` (indentation 2, `static`, noms `module_verbe`, déclarations en tête de bloc, `-std=c11 -pedantic -Werror`).
- Ne pas modifier `src/engine.h` : c'est le contrat entre les fiches. Si un changement est indispensable, l'écrire dans `tasks/NOTES-NN.md` au lieu de l'appliquer.
- Livrer, à la fin, un court `tasks/REPORT-NN.md` : fichiers créés, commandes de test lancées avec leur sortie, écarts avec le JS, points non faits.

## Contrats communs

### Format des pixels (C)

- Textures (`engine->tex[id]`) : `uint32_t` 0xAARRGGBB **non prémultiplié**, 64×64, indice `y * 64 + x`.
- Framebuffer (`engine->frame`) : 0x00RRGGBB (`CAIRO_FORMAT_RGB24`), 400×200, indice `y * 400 + x`.
- Un pixel de sprite est dessiné si alpha ≥ 128, comme `(c>>>24)<128` à la ligne 296 du JS.
- Le JS stocke du RGBA en octets (ABGR en u32 little endian). Ici c'est ARGB : `shade` et `bright` s'appliquent canal par canal, donc la formule reste la même.

### Ids de texture

L'`enum engine_tex` de `src/engine.h` est la source de vérité. Côté kc3, le module `DoomTex` (fiche 03) en reprend les valeurs une par une, dans le même ordre.

### Cellules de grille

Un `char` par cellule, comme dans les maps JS : `.` vide ; `1`-`6`, `D` (porte), `X` (sortie) solides.
Hors grille, la cellule est solide. Une porte ouverte redevient `.`.

### Vue par frame kc3 → C (fiche 05)

`Doom.view(state)` renvoie un tuple :

```
{state_sym, px, py, pa, floor_tex, ceil_tex, fog, sprites, hud, sfx}
```

- `state_sym` : `:title`, `:play`, `:pause`, `:dead`, `:inter` ou `:win`.
- `px`, `py`, `pa`, `fog` : F64. `floor_tex`, `ceil_tex` : entier. Le C accepte tout type entier, parce que kc3 redescend l'arithmétique entière.
- `sprites` : List de `{F64 x, F64 y, U32 tex, F64 scale, F64 lift, Bool flash}`.
- `hud` : Map `%{key: value}`. Clés et types dans la fiche 05, section « Contrat de la Map `hud` ».
- `sfx` : List de symboles (`:pistol`, `:shotgun`, `:chain`, `:hit`, `:die`, `:hurt`, `:pick`, `:door`, `:fire`, `:click`, `:win`) émis pendant la frame.

## Environnement kc3 (pour les fiches kc3)

Lancer un script kc3 depuis la racine de la worktree :

```sh
(cd ../kc3 && . ./env && cd - >/dev/null && ../kc3/kc3s/kc3s --load tests/NOM.kc3 --quit)
```

- `. ./env` doit être sourcé depuis la racine de kc3, car il utilise `$PWD`. Ne pas composer `LD_LIBRARY_PATH` à la main.
- Ne jamais lancer `kc3s` depuis un dossier qui contient un `kc3.dump`.
- Les `.kc3c` générés à côté des sources sont des caches, ignorés par git.

Pièges du langage, constatés sur ce projet :

1. Un struct n'est pas utilisable dans son propre `defmodule` (`struct_type not found`). Il faut le mettre dans un module séparé : `DoomState`, `DoomMob`…
2. Il faut un pin pour lier depuis une variable ou une constante de module : `x = ^ y`, `{a, b} = ^ t`, `[h | t] = ^ l`, `g = ^ Module.const`.
3. L'arithmétique entière redescend au plus petit type : `U64 + U64` donne un U8. Caster le résultat : `(U64) (a + 1)`.
4. `sqrt` ne gère pas F64. Utiliser `def fsqrt = cfn F64 "sqrt" (F64)`. `F64.cos`, `F64.sin` et `F64.tan` existent.
5. `! a && b` se lit `! (a && b)` : toujours écrire `(! a) && b`.
6. Qualifier tous les appels (`Doom.step(...)`, pas `step(...)`) : un nom non qualifié peut être capturé par une variable homonyme de l'appelant.
7. Les littéraux entiers sont U8 par défaut : caster (`(U32) 400`, `(S64) -1`, `(F64) 0.5`).
8. `require Time` avant `Time.now()`.
9. Un commentaire `## x` suivi d'une ligne vide avale le reste du fichier en `--load`. Ne pas mettre de commentaires dans les `.kc3`.
10. `1.0 / 0.0` ne donne pas un infini exploitable : tester `== 0.0` avant de diviser.
