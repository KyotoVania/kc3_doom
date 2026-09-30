# Lot 09 — sortir de Cairo (priorité haute)

Statut : objectif demandé par le propriétaire, non implémenté.

## Intention

Remplacer Cairo comme moteur de dessin et de présentation du jeu. Privilégier EGL
pour les contextes, surfaces et échanges de buffers. Évaluer OpenGL ES pour le dessin ;
si les capacités réelles de kc3 ou des plateformes l'imposent, documenter un chemin
OpenGL de repli avant de le retenir.

EGL n'est pas une API de dessin concurrente d'OpenGL : il assure l'interface avec la
plateforme native. Voir la [description Khronos](https://www.khronos.org/egl/).
Le couple exact d'API, sa version minimale et les plateformes supportées ne sont pas
encore décidés.

## Étapes et preuves attendues

| Étape | Travail | Critère de validation |
|---|---|---|
| 1. Audit | Identifier les backends EGL/GL, bindings et événements déjà disponibles dans kc3, ainsi que les dépendances et pilotes requis | Inventaire référencé ; réutilisation de kc3 avant ajout d'un adaptateur spécifique |
| 2. Prototype | Fenêtre, contexte, surface, dessin simple, swap, resize et fermeture | Exécution locale prouvée ; erreurs d'initialisation et nettoyage testés |
| 3. Frontière kc3/native | Garder DoomApp, état et décisions en kc3 ; définir un contrat de rendu indépendant de Cairo | Tests du contrat, aucune logique de jeu transférée en C pour cette migration |
| 4. Transition | Présenter d'abord l'image existante comme texture GPU si utile | Étape transitoire explicitement nommée : uploader une image ne transforme pas le raycaster CPU en rendu GPU |
| 5. Remplacement complet | Migrer textures, HUD, texte, composition et chemin de fenêtre dépendant de Cairo | Build du jeu sans dépendance Cairo ; comparaison des captures et comportements |
| 6. Mesures et CI | Mesurer CPU/GPU, temps de frame complet, démarrage et mémoire ; tester Mesa logiciel en CI si réalisable | Mesures avant/après, suite logique verte et tests de cycle de vie adaptés au nouveau backend |

## Contraintes

- Conserver Cairo comme référence de comparaison pendant la migration ; ne le retirer
  du build du jeu qu'après validation de la parité utile. Les anciens tests de référence
  peuvent rester séparés, sans être une dépendance du jeu final.
- Ne pas réécrire d'abord le HUD et les textures avec de nouvelles primitives Cairo
  spécifiques : coordonner leur expression en kc3 avec le futur contrat de rendu.
- Ne pas promettre un gain de performances sans mesurer toute la frame et les copies.
- Le rendu doit rester fonctionnel avec les données et règles kc3 existantes.
- Toute modification de `../window` ou `../libkc3` reste soumise à la branche dédiée,
  au test red→green, à ASan et à la validation humaine ; cette fiche ne les contourne pas.
- Aucun nouveau backend ni aucune suppression de Cairo n'est livré avec cette fiche.
