# Fenêtre et entrées pilotées en kc3

## Objectif

Faire choisir au programme kc3 la fenêtre, les callbacks et la séquence
`update → view → dessin`, en réutilisant les bibliothèques fenêtre de kc3.
Le backend natif conserve la boucle système et transporte un état kc3 opaque.

## Répartition du 29 septembre 2026

- Planification, intégration, revue et documentation : agent principal.
- Souris et régressions : Kimi K3, sur `kc3/game.kc3` et
  `tests/motion_test.kc3`. Reprise du début de patch Claude après épuisement
  de ses crédits.
- Audit fenêtre et backend de test sans affichage : Kimi K3.
- Adaptateur fenêtre, application et tests de contrat : Claude Sonnet après
  la période d'indisponibilité ; corrections d'intégration et validation par
  l'agent principal.

Lot implémenté. Résultats et limites : `REPORT-07.md`.

## Contrat visé

- `DoomApp` choisit titre, dimensions et callbacks.
- Les callbacks reçoivent l'état, une référence native temporaire de fenêtre
  et les arguments de l'événement ; ils retournent `{continuer, état}`.
  `:error` signale un échec, distinct d'un arrêt normal par `false`.
- L'adaptateur C valide le contrat, conserve les valeurs kc3 et libère ses
  ressources. Il ne connaît pas les règles ni la structure de l'état du jeu.
- Le callback kc3 de rendu décide du pas de temps et appelle successivement
  la mise à jour, la construction de vue et la primitive native de dessin.
- La souris utilise les différences entre coordonnées absolues. Le premier
  mouvement initialise la référence ; les transitions de jeu la réinitialisent.

## Validation

- Les six tests du handoff restent verts ; consigner leurs sorties.
- Tester le premier mouvement, les deux sens de rotation, les écrans inactifs
  et les transitions de niveau ou de redémarrage.
- Tester les callbacks et la séquence de rendu sans affichage.
- Vérifier la compilation stricte et, si possible, les chemins natifs de
  validation et de nettoyage sans serveur graphique.
- Distinguer dans le compte rendu ce qui est testé automatiquement de ce qui
  nécessite encore une partie interactive.

## Limites de ce lot

Pas de changement du runtime ni de `../window`, pas de commit. La capture
relative, le relâchement des touches, le son et le portage des textures/HUD
restent des lots suivants. La souris absolue reste limitée par les bords de
la fenêtre et ne constitue pas encore une visée FPS relative.

## Référence avant intégration

Les six tests ont passé avant ce lot. Mesure indicative d'une exécution :
6,284 ms par mise à jour à 30 mobs et 3,809 ms par vue. Ce lot est fonctionnel,
pas une optimisation ; aucune promesse de gain de performances.
