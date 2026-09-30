# Contribuer à kc3_doom

Merci de l'intérêt ! Ce dépôt a des règles strictes, héritées de son mode de
développement. Lire **`AGENTS.md` et `HANDOFF.md` en entier avant toute contribution** :
le second décrit les contrats internes, les mesures de référence et les pièges du
langage kc3 que vous rencontrerez forcément.

## Avant de coder

- Identifiez où vit votre changement : logique de jeu et application en kc3
  (`kc3/*.kc3`), primitives chaudes en C (`src/`). La tendance est de déplacer du C vers
  kc3 quand le coût est maîtrisé — pas l'inverse.
- Tout changement de contrat (`src/engine.h`, `src/hud.h`, tuple de vue, Map `hud`) se
  répercute sur `src/bridge.c` **et** `tests/view_check.kc3`.
- Le dépôt vit dans un checkout kc3 compilé (`kc3/kc3_doom/`). Vérifiez que
  `./configure && make` fonctionne avant de commencer.

## Règles de code

- **Aucun commentaire dans le code**, sauf l'en-tête de licence des fichiers C. Aucun
  commentaire du tout dans les `.kc3` (un `##` suivi d'une ligne vide avale le reste du
  fichier en `--load` — voir `HANDOFF.md` section 7).
- `libkc3/kc3.h` toujours inclus en premier dans un `.c` (conflit `bool`/`stdbool.h`).
- Style C de `src/` : indentation 2 espaces, `-std=c11 -pedantic -Werror`, déclarations
  en tête de bloc.
- Ne pas ajouter de dépendance sans discussion préalable.

## Tests (obligatoires)

- Les **six tests historiques** (`engine_test`, `textures_test`, `hud_test`,
  `data_test`, `game_test`, `view_check`) doivent rester verts. Lancer la suite
  complète depuis la racine du dépôt :

  ```sh
  sh tests/run_all.sh
  ```

  Joindre la sortie à la pull request. Le script vérifie les marqueurs `OK` et les
  lignes `FAIL`, pas seulement les codes de sortie.
- `game_test` affiche un `ms/frame` : c'est le benchmark de référence.
- **Toute optimisation se prouve** par une mesure avant/après (`ms/frame` de
  `game_test` ou micro-bench dédié joint à la PR). Aucun gain supposé n'est accepté,
  aucun pourcentage seul ne suffit.
- Ajoutez un test si vous changez un contrat ou une primitive ; calquez les tests
  existants dans `tests/`.

## Toucher au runtime kc3 (../libkc3, ../window)

Périmètre à part, hors merge direct :

- branche dédiée dans le dépôt kc3, **jamais sur `master`** ;
- test red→green démontrant le problème puis le correctif ;
- passe ASan (`make asan`, tests kc3) ;
- validation humaine du mainteneur avant tout merge ;
- ne jamais lancer `kc3s` depuis un dossier contenant un `kc3.dump`.

## Commits

- Convention : `[TAG] TITRE EN CAPS`, corps en minuscules.
- Aucune mention d'IA, d'outil ou de co-auteur automatique dans les messages.
- Commits atomiques : un lot, un commit.

## Issues et pull requests

- Bugs : utilisez le modèle « Rapport de bug » — reproduire avec `tests/run_all.sh`
  ou une commande précise, et collez les extraits de sortie pertinents.
- Propositions : utilisez le modèle « Demande de fonctionnalité » et indiquez si la
  fonctionnalité doit vivre côté kc3 ou côté C, avec un plan de mesure si elle touche
  aux performances.
- Le workflow CI préparé ne couvre que les tests C autonomes et le site : la suite complète
  reste à lancer localement et sa sortie fait partie de la preuve de la PR.
- Pour le site : `python3 -m unittest discover -s tests -p 'site_test.py' -v`.
  La procédure de publication est dans [.github/PUBLISHING.md](.github/PUBLISHING.md).
