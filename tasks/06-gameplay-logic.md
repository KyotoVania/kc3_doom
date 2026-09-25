# Fiche 06 : logique de jeu en kc3

**Prérequis** : la fiche 03 (`kc3/data.kc3`) est mergée. La fiche 05 peut tourner en parallèle.
Tant que `kc3/doom_engine.kc3` n'existe pas, tu utilises un stub.

## Contexte

`kmx_doom` porte en C et kc3 un FPS écrit en JS (`kmx_doom.html`). Toute la logique de jeu est en kc3.
Chaque frame, l'hôte C appelle `Doom.update(state, dt)` puis `Doom.view(state)`, et il transmet les entrées à `Doom.key`, `Doom.button` et `Doom.motion`.
Le rendu, le raycasting (`los`, `cast`) et les collisions (`blocked`) sont en C, exposés par le module `DoomEngine`.
Pour les performances, on a mesuré environ 21 µs par appel de fonction kc3, et une frame doit tenir en 16,6 ms, rendu compris. Ta logique doit rester autour de 2 ms par frame avec 30 mobs.
Lis `tasks/README.md` en entier, en particulier les 10 pièges kc3 et le contrat « Vue par frame ». Lis aussi `tasks/05-bridge.md`, section « Contrat de la Map `hud` ».

## À lire d'abord

- `kmx_doom.html`, lignes 226-379 (état, niveaux, collisions, tir, dégâts, usage, ramassage, `update`), 482-509 (états, clavier, souris), 443-453 (choix de la ligne de Thomas et des messages).
- `kc3/data.kc3` : tu utilises ces données sans les recopier.
- `kc3/doom.kc3` : le squelette actuel, à remplacer.
- `tasks/05-bridge.md` : la liste des `cfn` de `DoomEngine` et le contrat de la vue.

## À produire

### `kc3/game.kc3` : remplace `kc3/doom.kc3`

Supprimer `kc3/doom.kc3`, en supprimant seulement le fichier : pas de `git rm`, pas de commit.

Structs, chacun dans son propre module (piège 1) :
- `DoomPlayer` : champs de `newPlayer` (ligne 229) ;
- `DoomMob` : ligne 246 ;
- `DoomItem`, `DoomProj`, `DoomMsg` ;
- `DoomLevelState` : ligne 236 (`kills`, `total`, `facts`, `ftotal`, `time`, `boss` en index ou `void`, `intro`, `shot_t`, `def` en `%DoomLevel{}`, `li`) ;
- `DoomGlobal` : `G`, ligne 482 ;
- `DoomState` : `state` (Sym), `state_t`, `time`, `show_map`, `mouse_down`, `keys` (Map keysym → Bool), `player`, `snap`, `level`, `mobs`, `items`, `projs`, `thomas`, `msgs`, `tip_i`, `global`, `sfx`.

Module `Doom`, qui est l'API appelée par le C :

| Fonction | Rôle |
|---|---|
| `Doom.init(w, h)` | Joueur neuf, niveau 0 chargé, état `:title` (ligne 536). |
| `Doom.update(s, dt)` | Port de `frame` (ligne 539) : `time`, `state_t`, puis `update` si `:play`, ou rotation de la caméra si `:title`. Vider `sfx` au début. |
| `Doom.key(s, keysym)` | Port de `keydown` (lignes 496-502), avec les keysyms X11 (liste ci-dessous). Pas de `keyup` (voir plus bas). |
| `Doom.button(s, button, x, y)` | Port de `mousedown` et `click` (lignes 486-493, 507). Bouton 1 : tir en `:play`, sinon `click`. |
| `Doom.motion(s, x, y)` | Pas de souris relative pour l'instant. Renvoyer `s` tel quel. |
| `Doom.view(s)` | Construit le tuple du contrat (README, « Vue par frame ») et la Map `hud` (fiche 05). |

Keysyms X11 : flèches 65361 (gauche), 65362 (haut), 65363 (droite), 65364 (bas) ; `w` 119, `a` 97, `s` 115, `d` 100, `z` 122, `q` 113 (ZQSD en AZERTY) ; `e` 101 ; espace 32 ; Entrée 65293 ; `m` 109 ; Tab 65289 ; `1`, `2`, `3` : 49, 50, 51 ; Shift 65505 et 65506.

**Entrées sans relâchement de touche.** La fenêtre n'envoie pas encore d'événement « touche relâchée ».
- Déplacement par impulsion : chaque pression de flèche ou de ZQSD applique un pas de mouvement de 0.18 et une rotation de 0.09 rad.
- L'autorepeat X11 répète les pressions quand on garde la touche enfoncée.
- Préparer `keys` pour plus tard : ajouter `Doom.key_up(s, keysym)` qui met la touche à false. Si `keys` est mise à true par `Doom.key`, le mouvement continu se fait dans `update`. Garder les deux chemins derrière un booléen `DoomConfig.continuous_keys`, à false pour l'instant.
- Le tir continu (`mouseDown`) n'existe pas : un clic donne un tir.

Moteur : appeler uniquement `DoomEngine.los`, `DoomEngine.cast`, `DoomEngine.blocked`, `DoomEngine.grid_load`, `DoomEngine.grid_set` et `DoomEngine.grid_get`.
- `move` (ligne 250) se réécrit en kc3 au-dessus de `DoomEngine.blocked`, avec 2 appels.
- `load_level` fait `DoomEngine.grid_load(map)` une fois.
- `use` ouvre une porte avec `DoomEngine.grid_set(x, y, (U8) 46)` (46 = `.`).
- La séparation des mobs (ligne 375) coûte 35 ms par frame en kc3 : **ne pas la porter**, la remplacer par rien. Le noter dans le rapport comme candidate à une `cfn` C.

Aléatoire : `def rand = cfn F64 "drand48" ()` dans un module `DoomRand`, et `DoomRand.seed = cfn Void "srand48" (S64)`. Remplace `Math.random`.

Textures des sprites : `DoomMobTex.walk/atk/dead` et `DoomTex.*` de `kc3/data.kc3`.
- Clignotement des `race` (ligne 282) : ne pas émettre le sprite si `DoomRand.rand() < 0.25`.
- `lift` des items : `c[2] + sin(time * 3 + x) * 0.05` si `c[2]` n'est pas nul (ligne 283).

Sons : ajouter dans `s.sfx` les symboles émis par `sfx(...)` dans le JS.

### `tests/engine_stub.kc3`

Module `DoomEngine` de remplacement, en kc3 pur, avec les mêmes noms et arités que la fiche 05 :
- `los` renvoie true, `cast` renvoie `(F64) 96.0` ;
- `blocked` renvoie true hors du rectangle [1, 23] × [1, 17], false sinon ;
- `grid_load` et `grid_set` renvoient true ; `grid_get` renvoie `(U8) 46`.

### `tests/game_test.kc3`

Charge `kc3/data.kc3`, `tests/engine_stub.kc3` puis `kc3/game.kc3`, et joue une séquence scriptée :

1. `init`, puis `view` : `state_sym == :title` et `hud` contient toutes les clés du contrat de la fiche 05 ;
2. `button(1)` après `state_t` ≥ 0.5, obtenu par des `update` : l'état passe à `:play`, le niveau 0 est chargé et `total` vaut le nombre de mobs de la map E1M1 ;
3. 10 pressions haut : `px` ou `py` a changé ;
4. `key(50)` sans gdb : l'arme reste 0. Téléporter le joueur sur l'item `g` (modifier `s.player` directement), faire un `update` : `owned[1]` passe à true et `weapon` vaut 1 ;
5. forcer l'`hp` d'un mob à 1 puis tirer dessus en l'alignant (placer le joueur en face, `pa` calculé) : `kills` augmente de 1 et un message est ajouté ;
6. forcer `hp` du joueur à 1 puis `hurt_player` : l'état passe à `:dead`. `button(1)` : l'état repasse à `:play` et le niveau est rechargé ;
7. 60 `update` avec 30 mobs en alerte : afficher `ms/frame` avec `Time.now()` ;
8. si tout passe, afficher `game_test: OK`.

Lancer ainsi :

```sh
(cd ../kc3 && . ./env && cd - >/dev/null && ../kc3/kc3s/kc3s --load tests/game_test.kc3 --quit)
```

## Critères d'acceptation

- `game_test: OK`, aucune erreur `env_eval`, `buf_parse` ou `tag_to_pointer`.
- `ms/frame` de l'étape 7 inférieur à 3 ms. Joindre la valeur au rapport.
- Le rapport contient une table ligne JS → fonction kc3 couvrant les lignes 226-379 et 482-509, avec ce qui est porté, adapté ou omis, et pourquoi.

## Interdits

Ne pas modifier `src/`, `kc3/data.kc3` ni `../kc3`. Pas de `make`, pas de commit. Pas de `kc3s` depuis un dossier contenant un `kc3.dump`.

## Livrable

`kc3/game.kc3`, la suppression de `kc3/doom.kc3`, `tests/engine_stub.kc3`, `tests/game_test.kc3` et `tasks/REPORT-06.md`.
