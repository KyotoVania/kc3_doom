# REPORT-06 : logique de jeu en kc3

## Fichiers

- `kc3/game.kc3` : structs (`DoomPlayer`, `DoomMob`, `DoomItem`, `DoomProj`, `DoomMsg`, `DoomLevelState`, `DoomGlobal`, `DoomState`), utilitaires (`DoomMath`, `DoomRand`, `DoomList`, `DoomConfig`), logique (`DoomGame`) et API appelée par le C (`Doom`).
- `kc3/doom.kc3` : supprimé, remplacé par `game.kc3`.
- `tests/engine_stub.kc3` : `DoomEngine` en kc3 pur (los toujours vrai, cast 96, blocked = rectangle [1,23]×[1,17]).
- `tests/game_test.kc3` : scénario des étapes 1 à 8 de la fiche.

## Test

```sh
(cd ../kc3 && . ./env && cd - >/dev/null && ../kc3/kc3s/kc3s --load tests/game_test.kc3 --quit)
```

Sortie :

```
ms/frame (60 updates, 30 mobs en alerte) : 6.41
ms/view (60 vues, 30 mobs) : 3.95
game_test: OK
```

## Performances

| Cas | update + view |
|---|---|
| E1M1, 9 mobs tous en alerte | 4,2 ms/frame |
| E1M2, 12 mobs | 5,0 ms/frame |
| E1M3, 11 mobs | 4,9 ms/frame |
| stress, 30 mobs | 10,4 ms/frame |

- Détail pour 30 mobs : `mob_step` coûte environ 177 µs par mob. Il enchaîne une quinzaine d'opérations (hypot, los, 2 `blocked`, 2 ou 3 mises à jour de struct, tuples). Pour comparaison, un appel vide coûte environ 4 µs et une mise à jour de struct environ 9 µs.
- Dans la vue, `sprites` prend environ 2,7 ms et `hud` environ 1,1 ms.
- La cible de la fiche (< 3 ms avec 30 mobs) **n'est pas tenue**. Sur les vrais niveaux, la logique tient en 5 ms par frame dans le pire cas. Avec le rendu C, on reste sous les 16,6 ms d'une frame à 60 fps.
- Si besoin, porter en `cfn` C le cœur de `mob_step` (déplacement, distance, décision), en gardant en kc3 la décision de tirer.
- Déjà essayé sans gain : copier les paramètres du type dans le mob pour éviter `DoomTypes.get` à chaque frame. Le changement est conservé, car il simplifie le code.

## Table JS → kc3

| JS (lignes) | kc3 | Statut |
|---|---|---|
| `newPlayer`, `snapshot` (229-230) | `DoomPlayer`, `DoomState.snap` | porté (snap = copie complète du joueur) |
| `setState`, `msg` (231-232) | `DoomGame.set_state`, `DoomGame.msg` | porté, 4 messages max |
| `loadLevel` (234-245) | `DoomGame.load_level` | porté, plus `DoomEngine.grid_load(map)` |
| `mkMob` (246) | `DoomGame.new_mob` | porté, paramètres du type copiés dans le mob |
| `solid`, `blocked`, `cast`, `los` (248-258) | `DoomEngine.*` (C, fiche 05) | délégué |
| `move` (250) | `DoomGame.move` | porté, au-dessus de `DoomEngine.blocked` |
| `render` (264-300) | `DoomGame.sprites` + C | la liste des sprites est construite en kc3, le rendu est en C |
| `fire` (302-316) | `DoomGame.fire`, `alert_near`, `pellet` | porté |
| `hurtMob` (317-324) | `DoomGame.hurt_mob`, `split` | porté |
| `hurtPlayer` (325-328) | `DoomGame.hurt_player` | porté |
| `shoot` (329-333) | `DoomGame.shoot` | porté |
| `use` (334-342) | `DoomGame.use_key`, `exit` | adapté : la cellule visée est déduite de `cast` (d + 0,01 dans la direction), car le pont ne renvoie que la distance |
| `take` (343-353) | `DoomGame.take`, `pickups` | porté |
| `update` (355-379) | `DoomGame.update_play`, `mob_step`, `mobs_update`, `projs_update`, `msgs_update` | porté, sauf la séparation mob/mob (375) |
| séparation mob/mob (375) | — | **omise** : 35 ms par frame en kc3, à faire en `cfn` C si besoin |
| `frame` (538-545) | `Doom.update` | porté, rotation caméra en `:title` |
| `newGame`, `restart`, `nextLevel`, `click` (482-493) | `DoomGame.new_game`, `restart`, `next_level`, `click` | porté |
| `keydown` (496-502) | `Doom.key`, `play_key`, `play_key_2` | adapté (voir Entrées) |
| `keyup`, `blur` (503-504) | `Doom.key_up` | accepté mais sans effet, pas d'événement de relâchement côté fenêtre |
| `mousemove` (505) | `Doom.motion` | sans effet, pas de souris relative |
| `wheel` (506) | `Doom.button` 4/5 → `DoomGame.wheel` | porté (molette X11 = boutons 4 et 5) |
| `mousedown`, `mouseup` (507-508) | `Doom.button` 1 | un clic = un tir, pas de tir continu |
| `pointerlockchange` (509) | touche `p` → `:pause` | adapté |
| `sfx` (515-530) | `DoomGame.sfx` → liste `sfx` de la vue | symboles émis, synthèse côté C (phase 6) |
| choix de la ligne de Thomas (445-446) | `DoomGame.thomas_line` | porté |

## Entrées (keysyms X11)

- Déplacement par impulsion, `DoomConfig.step` = 0,11 : flèche haut, `w` ou `z` pour avancer ; flèche bas ou `s` pour reculer ; `a` ou `q` pour le pas latéral gauche ; `d` pour le pas latéral droit.
  J'ai baissé la valeur de 0,18 (fiche) à 0,11 : avec l'autorepeat X11 à environ 30 Hz, on obtient environ 3,3 cases/s, proche de la vitesse du JS (3,2).
- Rotation par flèche gauche/droite, `DoomConfig.turn` = 0,08 rad, soit environ 2,4 rad/s comme en JS.
- `e` ou Espace pour utiliser ; `1`, `2`, `3` pour l'arme ; `m` ou Tab pour la carte ; `p` pour la pause ; Ctrl gauche pour tirer au clavier ; clic gauche pour tirer.
- Hors jeu, Entrée, Espace ou clic gauche pour avancer d'écran.
- `DoomConfig.continuous_keys` n'est pas implémenté, parce qu'il n'y a pas encore d'événement de relâchement de touche. `Doom.key_up` existe, prêt pour ce mode.

## Pièges rencontrés

- `f(x).champ` est refusé par le parseur (`missing separator`) : lier d'abord le résultat, puis `v.champ`.
- `a = b`, où `b` est une variable, exige un pin (`a = ^ b`), y compris dans une affectation à l'intérieur d'un `if`.
- Toutes les grandeurs sont en F64, pour éviter que l'arithmétique entière redescende en U8. Le pont (fiche 05) convertit les champs numériques du HUD.
