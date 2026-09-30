# kc3_doom

Un jeu de tir à la première personne **inspiré de Doom**, écrit autant que possible en
[kc3](https://github.com/kc3-lang/kc3) (langage et runtime du même écosystème), avec un moteur de rendu
logiciel en C (raycasting, Cairo, XCB).

L'ambition : refaire un jeu façon Doom en kc3, avec des niveaux et des textures
procédurales propres au projet. Ce n'est **pas un port officiel du moteur Doom** ;
aucune compatibilité avec ses fichiers de jeu n'est annoncée. C'est un terrain de jeu
et de mesure pour pousser du code réel en kc3 et, à terme, améliorer le runtime lui-même.

[Visite guidée du code](presentation-code.html) · [État technique](HANDOFF.md) ·
[Contribuer](CONTRIBUTING.md)

[Dépôt GitHub](https://github.com/KyotoVania/kc3_doom) ·
[GitHub Actions](https://github.com/KyotoVania/kc3_doom/actions/workflows/ci.yml)

## Positionnement

`kmx_doom.html` (à la racine) est le **prototype JavaScript historique en un seul fichier**
qui sert de référence fonctionnelle : comportements des mobs, armes, niveaux, HUD. La
finalité du projet est le jeu natif kc3/C décrit ici, pas une page web. Le dépôt
ne fait **pas tourner le jeu kc3 dans le navigateur** : le site prévu (GitHub Pages) publiera une
présentation HTML et des liens vers les sources, rien de plus.

## État actuel

- Trois niveaux jouables (E1M1, E1M2, E1M3 avec boss).
- Pilotage de l'application en kc3 : choix de la fenêtre, callbacks clavier/souris,
  pas de temps borné, séquence `update → view → draw`, arrêt sur Échap.
- Rendu : raycaster, 47 textures procédurales, HUD complet (arme, visage, minimap).

Les tests sans affichage passent localement. Une validation interactive complète reste
à faire après le transfert du pilotage fenêtre et souris en kc3.

Limitations connues (détails dans `HANDOFF.md`, section 6) :

- pas de relâchement de touche : le déplacement se fait par impulsions via l'autorepeat ;
- pas de souris relative ni de capture du pointeur : la visée est limitée par les bords
  de la fenêtre ;
- pas de son (la vue émet une liste `sfx`, ignorée côté C) ;
- séparation mob/mob non portée ;
- un clic = un tir (pas de tir continu).

## Répartition kc3 / C

La frontière est honnête et assumée :

- **kc3** : logique de jeu (IA, tirs, dégâts, portes, ramassages, états), données des
  niveaux et des armes, entrées, orientation de la caméra, pilotage complet de
  l'application et des callbacks fenêtre ;
- **C** : raycasting et primitives géométriques, adaptateur vers le backend fenêtre
  Cairo/XCB de kc3 ; textures et HUD sont aussi en C par choix d'implémentation initial,
  pas par nécessité. La logique kc3 appelle le moteur
  via des `cfn` (`DoomEngine.los`, `cast`, `blocked`, `grid_*`).

La tendance de fond est de déplacer du C vers kc3 à coût maîtrisé (textures et HUD sont
les prochains candidats), mesures à l'appui.

### Flux d'une frame

```
xcb EXPOSE → adaptateur natif → DoomApp.on_render(app, window, dt)
  DoomApp.clamp(dt)                 kc3 : borne le pas de temps à [0, 0,05]
  Doom.update(app.game, dt)         kc3 : IA, projectiles, ramassages, timers
  Doom.view(game)                   kc3 : tuple de vue
  DoomEngine.draw(window, view)     cfn : validation, raycasting, blit et HUD
  {true, nouvel_app}                kc3 : état rendu à l'adaptateur
```

## Priorités de développement

1. **Remplacer Cairo par un rendu GPU**, avec EGL comme voie privilégiée pour les
   contextes et surfaces. Évaluer OpenGL ES et ne retenir un chemin OpenGL de repli
   qu'avec une contrainte de compatibilité documentée. [Cadrage du lot](tasks/09-rendu-egl.md).
2. Conserver les règles, le pilotage et les décisions de dessin en kc3 ; adapter la
   migration textures/HUD à ce nouveau backend plutôt que renforcer la dépendance Cairo.
3. Compléter les entrées (relâchement de touche, souris relative), puis le son.

Cette migration est **planifiée, pas implémentée** : le build actuel dépend encore de
Cairo/XCB. EGL ne dessine pas à lui seul ; il relie une API de rendu à la plateforme
native ([Khronos](https://www.khronos.org/egl/)).

## Arborescence

| Fichier | Rôle |
|---|---|
| `src/kmx_doom.c` | lanceur : initialise kc3, charge les modules, appelle `DoomApp.main` |
| `src/window_binding.c/.h` | adaptateur du backend Cairo/XCB (événements, état opaque, nettoyage) |
| `src/bridge.c/.h` | primitives natives du moteur, validation et dessin de la vue |
| `src/engine.c/.h` | raycaster, grille, collisions |
| `src/textures.c` | textures procédurales (RNG identique au prototype JS) |
| `src/hud.c/.h` | HUD, arme, visage, minimap, écrans |
| `kc3/game.kc3` | logique de jeu |
| `kc3/data.kc3` | niveaux, mobs, armes, ids de texture |
| `kc3/doom_app.kc3` | application : fenêtre, callbacks, boucle de frame |
| `kc3/doom_app_state.kc3` | état typé de l'application |
| `kc3/doom_engine.kc3` | déclarations des primitives natives |
| `kc3/doom_window.kc3` | binding `DoomWindow.run` |

## Build

Prérequis **exigeant** : le dépôt doit vivre dans un checkout de kc3 **déjà configuré et
compilé** (libkc3, kc3s, window/cairo/xcb). `./configure` récupère les drapeaux de
`../window/cairo/xcb/demo/config.mk` ; il échoue si le parent n'est pas prêt. Il faut
aussi Cairo, pkg-config et un environnement X11/XCB.

```sh
cd kc3/kc3_doom
./configure
make
make run               # à lancer depuis kc3_doom/ : chemins .kc3 relatifs
```

`KC3=/chemin/vers/kc3 ./configure` permet de sélectionner un arbre de compilation,
mais ne résout pas à lui seul les chemins de recherche du runtime. Pour la procédure
testée, conserver l'arborescence imbriquée ci-dessus. Voir aussi les
[instructions de compilation de kc3](https://github.com/kc3-lang/kc3#readme).

À l'écran titre : Entrée, Espace ou clic gauche pour commencer. En jeu : clic gauche
ou Ctrl gauche pour tirer, souris pour orienter la vue, Échap pour quitter.

## Tests

Suite complète (locale, nécessite le checkout kc3 parent) :

```sh
sh tests/run_all.sh
```

Elle couvre les six tests historiques (trois tests C autonomes — moteur, textures, HUD —
et trois tests kc3 — données, logique de jeu, contrat de vue), plus les tests de
callbacks applicatifs et les cas d'erreur natifs. `game_test` affiche un `ms/frame` qui
sert de **benchmark de référence** : toute optimisation doit le faire baisser, mesures à
l'appui. Les tests `game_test` et `view_check` tournent sans le C via un stub kc3 pur.

Le [workflow CI](.github/workflows/ci.yml) vérifie les tests C autonomes sous GCC et
Clang, la présentation et la génération du site. Il ne reconstruit pas encore
kc3 sur un runner vierge. La suite complète reste donc une étape locale. Les résultats
des exécutions distantes sont disponibles dans l'onglet GitHub Actions.

Sans runtime kc3, les tests C se lancent avec `sh tests/run_native.sh` (compilateur C,
pkg-config et Cairo requis).

## Performances

Quelques mesures (kc3s release, Linux, détails et méthode dans `HANDOFF.md` section 5) :
`update` + `view` coûtent environ 4 à 5 ms/frame sur les vrais niveaux, ~10 ms en stress
à 30 mobs. Le coût par appel kc3 (~4 µs la fonction vide) est le levier d'optimisation
identifié côté runtime. Ce sont des mesures historiques de logique et de vue, pas un
benchmark du rendu complet ni une promesse de FPS. Aucune optimisation n'est acceptée
sans mesure avant/après.

## Documentation

- [HANDOFF.md](HANDOFF.md) — état des lieux complet : contrats, mesures, pièges, pistes ;
- [PLAN.md](PLAN.md) — plan historique, certains contrats ont évolué ;
- [tasks/](tasks/) — fiches de lots et rapports ;
- [presentation-code.html](presentation-code.html) — visite HTML du code ; sur GitHub,
  ce lien affiche le source, ouvrir le fichier localement ou utiliser le futur site Pages ;
- [Publication GitHub et Pages](.github/PUBLISHING.md) — activation du site, périmètre CI
  et contrôles avant publication.

## Contribution

Voir [CONTRIBUTING.md](CONTRIBUTING.md). En résumé : lire `AGENTS.md` et `HANDOFF.md` d'abord, garder les
six tests verts, prouver toute optimisation par une mesure, respecter la convention de
commits.

## Licence et mentions

Copyright 2022–2026 **kmx.io** et copyright 2026 **KyotoVania**.

Le code, les tests, les scripts, la documentation et les fichiers HTML du projet sont
couverts par [LICENSE](LICENSE), sauf mention spécifique contraire. Le texte de
permission déjà présent dans les sources kc3 est conservé : les mentions de copyright
et de permission doivent accompagner les copies et portions substantielles ; le
logiciel est fourni « AS-IS », sans garantie.

Les crédits et mentions existants sont conservés. Les dépendances tierces gardent
leurs licences et leurs ayants droit. Voir [NOTICE](NOTICE).

Doom est une référence d'inspiration : ce projet n'est ni affilié à ses ayants droit,
ni approuvé par eux, et n'est pas un port officiel de son moteur.
