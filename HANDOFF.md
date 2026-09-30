# HANDOFF kc3_doom (30/09/2026)

Document de départ : `PLAN.md` (plan historique en 6 phases, conservé pour les décisions techniques).
Objectif du projet : **refaire un jeu inspiré de Doom en kc3**, avec une place croissante pour kc3 dans l'application et le jeu. Ce n'est ni un port officiel de Doom, ni un projet défini par la conversion du prototype HTML. Ce fichier fait le point et prépare la suite : rendre le jeu **le plus kc3 possible** et mesurer les optimisations, côté jeu ou runtime.

## 1. État

- Trois niveaux : E1M1 libkc3, E1M2 httpd, E1M3 PROD avec le boss `env->err`. Lancement : `./configure && make && make run` depuis `kc3/kc3_doom/`.
- **Lot 07 implémenté :** `DoomApp` choisit la fenêtre, les callbacks, le pas de temps et la séquence mise à jour/vue/dessin en kc3. La souris absolue oriente maintenant la caméra en kc3. Le backend fenêtre de kc3 est réutilisé sans modification du runtime.
- Le lot 07 est validé par les tests sans affichage ; une partie interactive reste à faire. Détails, sorties et limites : `tasks/REPORT-07.md`.
- Lot 08 : README, contribution, modèles GitHub, CI C/site et workflow Pages préparés. Le dépôt public est désormais `https://github.com/KyotoVania/kc3_doom`. Licence et mentions harmonisées au nom de kmx.io et KyotoVania ; publication du code autorisée par le propriétaire. Voir `LICENSE`, `NOTICE`, `tasks/REPORT-08.md` (rapport historique) et `.github/PUBLISHING.md`. La CI n'inclut pas encore le build complet de kc3 sur un runner vierge.
- Référence historique : `kmx_doom.html`, prototype JS en un seul fichier. Les numéros de ligne JS cités ici renvoient à ce fichier. `presentation-code.html` est la vitrine documentaire prévue pour GitHub Pages, pas une version navigateur du jeu kc3.
- Publication du site : branche orpheline `gh-pages`, contenant uniquement `index.html`, `.nojekyll`, `LICENSE` et `NOTICE`. La CI de `master` génère puis pousse le site sans force-push et déploie le contenu de cette branche via Actions. Source éditable : `presentation-code.html` sur `master`. Site : `https://kyotovania.github.io/kc3_doom/`.
- Toutes les phases de `PLAN.md` sont faites, sauf :
  - la phase 1 (patches `window/` : relâchement de touche, souris relative) ;
  - la phase 6 (son).
- Dépôt git autonome, imbriqué dans `kc3/` et exclu du dépôt kc3 par `kc3/.git/info/exclude`.

## 2. Arborescence et répartition

| Fichier | Rôle |
|---|---|
| `src/kmx_doom.c` | lanceur : initialise kc3, charge les modules et appelle `DoomApp.main` |
| `src/window_binding.c/.h` | adaptateur du backend Cairo/XCB : transporte les événements, valide les retours, conserve l'état opaque et nettoie les ressources |
| `src/bridge.c/.h` | primitives natives du moteur, initialisation/nettoyage, validation et dessin de la vue |
| `src/engine.c/.h` | raycaster, grille, collisions |
| `src/textures.c` | 47 textures procédurales, RNG mulberry32 identique au JS |
| `src/hud.c/.h` | HUD, arme, visage, minimap, bulle de Thomas, écrans |
| `kc3/game.kc3` | logique : IA, tir, dégâts, portes, ramassages, états, entrées, orientation souris et vue |
| `kc3/data.kc3` | niveaux, types de mobs, armes, astuces, ids de texture |
| `kc3/doom_engine.kc3` | module `DoomEngine` : primitives natives du moteur |
| `kc3/doom_app.kc3` | choix de fenêtre et callbacks, initialisation du jeu, limitation du pas de temps, `update → view → draw`, arrêt sur Échap |
| `kc3/doom_app_state.kc3` | état typé de l'application |
| `kc3/doom_window.kc3` | déclaration du binding `DoomWindow.run` |

Comptage documentaire du 30/09 après le lot 07 : **C 2 935 lignes / kc3 1 419 lignes**, soit environ 67,4 % / 32,6 %, hors lignes vides et commentaires, sur `src/*.[ch]` et `kc3/*.kc3` uniquement. Tests, runtime et prototype HTML sont exclus. Ce volume ne mesure pas qui décide des règles du jeu. Le lot 07 transfère le pilotage en kc3 mais ajoute un adaptateur C ; textures et HUD restent les prochains candidats à migrer.

## 3. Flux d'une frame

```
xcb EXPOSE → adaptateur natif → DoomApp.on_render(app, window, dt)
  DoomApp.clamp(dt)                 kc3 : borne le pas de temps à [0, 0,05]
  Doom.update(app.game, dt)         kc3 : IA, projectiles, ramassages, timers
  Doom.view(game)                   kc3 : tuple de vue inchangé
  DoomEngine.draw(window, view)     cfn : validation, raycasting, blit et HUD
  {true, nouvel_app}                kc3 : état rendu à l'adaptateur
```

- Les callbacks enregistrés depuis `DoomApp` reçoivent `(app, window, ...)` et retournent `{Bool continuer, nouvel_app}`. `false` signifie un arrêt normal ; `:error` ou un retour mal formé fait échouer `DoomWindow.run`. Le nettoyage valide aussi le retour du callback `unload`.
- Les callbacks kc3 de clavier, bouton et mouvement délèguent à `Doom.key`, `Doom.button` et `Doom.motion`. L'état reste une valeur kc3, conservée comme un `s_tag` opaque dans l'adaptateur C. Le pointeur de fenêtre ne doit pas être conservé au-delà de `DoomWindow.run` ; le contexte Cairo est utilisable uniquement pendant les callbacks avant nettoyage.
- Pendant `update`, la logique kc3 appelle le moteur par `DoomEngine.los`, `cast`, `blocked`, `grid_load`, `grid_set` et `grid_get`. Ce sont des `cfn` résolues par `dlsym(RTLD_DEFAULT)` sur l'exécutable, lié avec `--export-dynamic`.

Contrats détaillés :
- `src/engine.h` : ids de texture, `s_engine`.
- `src/hud.h` : `s_hud`.
- `tasks/README.md`, section « Vue par frame » : le tuple de vue.
- `tasks/05-bridge.md`, section « Contrat de la Map `hud` » : les clés du HUD.

## 4. Build, lancement, tests

```sh
cd kc3/kc3_doom
./configure            # reprend CPPFLAGS/CFLAGS de ../window/cairo/xcb/demo/config.mk (HAVE_F80, F80_SIZE…)
make && make run       # lancer depuis kc3_doom/ (chemins .kc3 relatifs, lib/kc3/0.1 trouvé via ../)
```

Suite complète, validée le 30/09 (Linux, avec `timeout` et `LD_PRELOAD`) :

```sh
sh tests/run_all.sh
```

Ce lanceur vérifie aussi les marqueurs `OK` et les lignes `FAIL`, car les anciens scripts kc3 ne garantissent pas un code de sortie non nul en cas d'échec. Les journaux sont dans `tests/out/`. Il couvre les six tests historiques, `motion_test`, `app_test`, les callbacks natifs sur surface Cairo hors écran, les fermetures et les retours invalides.

Commandes historiques des six tests :

```sh
for t in engine textures hud; do
  cc -std=c11 -W -Wall -Werror -pedantic -O2 $(pkg-config --cflags cairo) -Isrc \
     src/$t.c tests/${t}_test.c -o tests/out/${t}_test $(pkg-config --libs cairo) -lm \
  && ./tests/out/${t}_test | tail -1
done
for t in data_test game_test view_check; do
  (cd .. && . ./env && cd - >/dev/null && ../kc3s/kc3s --load tests/$t.kc3 --quit) | tail -1
done
```

- `game_test` et `view_check` tournent sans le C : `tests/engine_stub.kc3` remplace `DoomEngine` en kc3 pur.
- `game_test` affiche aussi un `ms/frame`. **C'est le benchmark de référence** : toute optimisation doit le faire baisser, mesures à l'appui.
- `./kmx_doom --load fichier.kc3` charge une surcharge après les modules habituels, puis appelle `DoomApp.main`. Le test natif de contrat remplace cette fonction ; ce drapeau n'est pas équivalent au mode script de `kc3s`.

## 5. Performances mesurées (kc3s release, Linux)

| Mesure | Valeur |
|---|---|
| appel de fonction kc3 vide | ~4,3 µs |
| mise à jour de struct `%T{s \| …}` | ~9 µs |
| itération de boucle `while` avec déstructuration | ~21 µs |
| `mob_step` (un mob, une frame) | ~177 µs |
| `update` + `view`, vrais niveaux, tous les mobs en alerte | 4,2 à 5,0 ms/frame |
| `update` + `view`, stress à 30 mobs | 10,4 ms/frame |
| `los` (DDA) en kc3 / en `cfn` C | ~475 µs / négligeable |
| séparation mob/mob O(n²) en kc3 (30 mobs) | 35 ms/frame (non portée) |

## 6. Limitations connues

- **Pas de relâchement de touche** dans `window/` (xcb ne gère que `XCB_KEY_PRESS`, `window/cairo/xcb/window_cairo_xcb.c:79`). Le déplacement se fait donc par impulsions via l'autorepeat. `Doom.key_up` existe, mais rien ne l'appelle.
- **Pas de souris relative ni de capture du pointeur.** `Doom.motion` utilise maintenant les différences de coordonnées absolues pour orienter la caméra. Le premier mouvement et les transitions de jeu réinitialisent la référence. La visée reste limitée par les bords de la fenêtre ; perte/reprise de focus et confort en jeu restent à valider.
- **Pas de son.** kc3 émet bien la liste `sfx` dans la vue, mais le C l'ignore.
- **Séparation des mobs non portée** (ligne 375 du JS).
- **Un clic = un tir** : pas de tir continu.
- Ctrl gauche tire au clavier.

## 7. Pièges rencontrés (langage et runtime kc3)

1. Un struct n'est pas utilisable dans son propre `defmodule` : il faut un module séparé.
2. Un pin est obligatoire pour lier depuis une variable : `a = ^ b`, `{x, y} = ^ t`, `[h | t] = ^ l`, `g = ^ Mod.const`.
3. L'arithmétique entière redescend au plus petit type (`U64 + U64` donne un U8). D'où le choix de F64 partout dans `game.kc3`.
4. `sqrt` (`tag_sqrt`, `libkc3/tag_sqrt.c:31-42`) ne gère pas F32/F64. On passe par `cfn F64 "sqrt" (F64)`.
5. `f(x).champ` est refusé par le parseur (`missing separator`) : il faut lier le résultat d'abord.
6. `! a && b` se lit `! (a && b)`. Il faut aussi qualifier tous les appels, sinon un nom non qualifié peut être capturé par une variable homonyme de l'appelant.
7. `KC3_DIR` avec un chemin absolu est inutilisable : `file_search` (`libkc3/file.c:1141-1160`) préfixe chaque entrée par `argv0_dir`. C'est pour ça que le dépôt vit dans `kc3/`.
8. libkc3 définit `typedef u8 bool` en C11 (`libkc3/types.h:129`), ce qui entre en conflit avec `<stdbool.h>`. Il faut toujours inclure `libkc3/kc3.h` en premier. `engine.h` et `hud.h` protègent `stdbool` derrière `LIBKC3_TYPES_H`.
9. Un `## commentaire` suivi d'une ligne vide avale le reste du fichier en `--load`. Pas de commentaires dans les `.kc3`.
10. Il n'y a pas de générateur aléatoire numérique dans la stdlib : `cfn F64 "drand48" ()` fonctionne.
11. Dans ce runtime, `tag_init_copy` ne remet pas à zéro les champs de comptage et de verrouillage. Initialiser les `s_tag` temporaires à `{0}` avant copie : une valeur non initialisée dans le nouvel adaptateur provoquait un blocage dans `tag_clean`.
12. Un champ de struct initialisé à `:none` est typé comme symbole. Pour stocker le jeu, `DoomAppState.game` est initialisé avec `%DoomState{}`.
13. `kc3_init` consomme `argv[0]` : les arguments restants commencent à l'indice zéro après son retour.

Candidats à remonter au mainteneur : les points 4 et 7, et peut-être le 5.

## 8. Pistes pour la suite (« plus kc3 » et optimisations)

Classées par levier. Chaque piste doit se mesurer avant/après avec `game_test` (ms/frame) ou un micro-bench dédié.

**Nouvelle priorité explicite du propriétaire : sortir de Cairo vers un rendu GPU,
EGL privilégié, chemin OpenGL de repli seulement si nécessaire.** EGL gère les
contextes/surfaces ; le choix de l'API de dessin (OpenGL ES ou OpenGL) reste à valider
par un prototype et l'audit des backends kc3 existants. Aucun backend n'a encore été
migré. Le lot `tasks/09-rendu-egl.md` prime sur la réécriture des dessins Cairo :
éviter d'investir dans des bindings Cairo spécifiques voués à disparaître.

### A. Runtime kc3 : le coût par appel (gros levier, profite à tout kc3)

- **Constat vérifié dans le source.** Chaque évaluation d'appel passe par `env_eval_call_resolve` (`libkc3/env_eval.c:~750-795`). Celle-ci appelle `env_module_has_ident`, puis `env_call_get` (`libkc3/env.c:349`), qui fait au moins **deux requêtes dans la base de facts** (`facts_find_fact_by_tags`, puis `facts_with_tags` et un curseur) sous `rwlock_r`. Rien n'est mémoïsé.
- L'audit `docs/audits/fixed/audit_kc3_httpd_capacite.md` (section 8, point 1) mesurait 62 % du CPU du httpd dans ce chemin. Il recommande de **mémoïser `env_call_get`** (ident → callable, invalidé sur `def` ou redéfinition), avec un gain estimé de ×2 à ×3 sur tout code kc3. Le point 2 propose un hachage non cryptographique à la place de SHA1 pour les hash de tags (17 % du CPU).
- Pour le jeu, c'est directement les ~177 µs de `mob_step` et les 4 à 5 ms par frame.
- **C'est du code critique de l'interpréteur** : il faut une discussion avec le mainteneur avant tout patch, un micro-bench red→green et une passe ASan (`make asan`, tests kc3). Travailler sur une branche kc3 séparée, jamais sur `master`.

### B. Déplacer du C vers kc3 à coût maîtrisé (pour augmenter la part de kc3)

- **Textures en kc3** : c'est le plus gros gain de pourcentage, puisque `textures.c` fait 993 lignes. Elles ne sont générées **qu'une fois au chargement**, donc le coût par appel ne pèse pas en jeu.
  - Il suffit d'exposer par `cfn` des primitives cairo sur une surface de texture courante (`rect`, `arc`, `curve`, `text`, `set_rgba`, `fill`, `stroke`, gradient radial), puis de réécrire les dessins de `textures.c` en kc3.
  - Estimation à vérifier : environ 50 primitives par texture × 47 textures × ~5-20 µs, soit **quelques dizaines de ms au démarrage**.
  - Garder le RNG mulberry32 identique. Le calcul sur des entiers 32 bits en kc3 est à tester, à cause de la réduction de type des entiers. Sinon, passer par une `cfn`.
  - Test d'égalité : comparer `tex[]` avec la version C, octet pour octet, via un test autonome.
- **HUD en kc3** : il y a environ 60 à 150 primitives par frame selon l'état. À ~5 µs l'appel, ça fait environ 0,5 à 1 ms par frame : acceptable, mais à mesurer. Une alternative plus rapide consiste à construire en kc3 une liste de commandes de dessin, que le C exécute en une seule passe.
- **Données en facts** : décrire les niveaux, mobs et armes en facts kc3 (`Facts`), interrogés au chargement du niveau. C'est très idiomatique kc3, et sans coût en jeu.
- **Garder en C** : le raycaster, `los`, `cast` et `blocked`, qui bouclent sur des pixels ou des cases.

### C. Optimisations côté jeu kc3

- **Mobs en `Array`** plutôt qu'en `List` : `DoomList.nth` et `set_nth` sont en O(n) dans `hurt_mob` et `fire`. À mesurer.
- **Réduire les mises à jour de struct dans `mob_step`**, qui en fait 2 ou 3 par mob et par frame, à ~9 µs chacune : calculer d'abord les champs, puis faire une seule mise à jour.
- **Transformer `sprites` et `hud`** (≈ 2,7 ms + 1,1 ms à 30 mobs) : les construire de façon incrémentale, ou ne recalculer la Map `hud` que si elle change.
- **Si un seul levier suffit** : porter en `cfn` C le cœur de `mob_step` (distance, déplacement, décision) et garder en kc3 la décision de tirer et les messages. Ça va contre l'objectif « plus kc3 » : à n'utiliser qu'en dernier recours.
- **Séparation des mobs** en `cfn` C (`DoomEngine.separate(List) -> List`), un seul appel par frame.

### D. Fonctionnalités manquantes

- **Relâchement de touche** dans `window/` (upstream kc3) : `XCB_EVENT_MASK_KEY_RELEASE` et un callback `key_release`.
  - Piège : `s_window_cairo` est casté en `s_window`. Il faut ajouter le champ au même rang dans **tous** les backends, ou bien en fin de struct partout.
  - Gérer aussi l'autorepeat X11.
- **Souris relative** (`xcb_grab_pointer` et `xcb_warp_pointer`), puis **son** (sndio ou ALSA) à partir de la liste `sfx`.

## 9. Règles de travail sur ce dépôt

- **Aucun commentaire dans le code**, sauf l'en-tête de licence des fichiers C.
- **Style C** de `src/` : indentation 2, `-std=c11 -pedantic -Werror`, déclarations en tête de bloc, `libkc3/kc3.h` inclus en premier.
- **Commits** : convention `[TAG] TITRE EN CAPS` + corps en minuscules, **sans aucune trace d'IA** (pas de `Co-Authored-By`).
- **Tests** : les 6 tests de la section 4 doivent rester verts. Tout changement de contrat (`engine.h`, `hud.h`, tuple de vue, Map `hud`) se répercute sur `bridge.c` et sur `tests/view_check.kc3`.
- **Runtime kc3** (`../libkc3`) : branche dédiée, red→green, ASan, et validation du mainteneur avant tout merge. Ne jamais lancer `kc3s` depuis un dossier qui contient un `kc3.dump`.
