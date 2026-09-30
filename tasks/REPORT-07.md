# Lot 07 — fenêtre et souris pilotées en kc3

Finalisé le 30 septembre 2026. Modifications locales, sans commit et sans
changement de `../libkc3` ou `../window`.

## Résultat

`DoomApp.main` choisit le titre, les dimensions et les sept callbacks.
Les fonctions kc3 décident de l'initialisation, du pas de temps, de la
séquence `Doom.update → Doom.view → DoomEngine.draw` et de l'arrêt sur Échap.
Le lanceur C charge les modules et appelle l'application ; l'adaptateur
fenêtre réutilise le backend Cairo/XCB existant.

L'adaptateur conserve un état kc3 opaque. Chaque callback reçoit cet état,
la fenêtre et les arguments de l'événement, puis retourne
`{Bool continuer, nouvel_état}`. `false` est un arrêt normal ; `:error`, un
retour mal formé ou une erreur d'évaluation fait échouer l'appel. Le retour
du callback de nettoyage est également validé.

La souris oriente la caméra en kc3 à partir de différences de coordonnées
absolues. Le premier événement initialise la référence sans rotation.
Les changements d'état du jeu réinitialisent cette référence.

Le raycaster, les recettes des textures, le HUD et les primitives natives
restent en C. Ce lot transfère les décisions d'application vers kc3 ; il
ne prétend ni supprimer tout hôte C, ni augmenter fortement le pourcentage
de lignes kc3. La migration des textures et du HUD reste à faire.

## Validation

Commande exécutée avec succès le 30 septembre :

```sh
sh tests/run_all.sh
```

Sorties des six tests historiques :

```text
engine_test: OK
textures_test: OK
hud_test: OK
data_test: OK
game_test: OK
view_check: OK
```

Tests supplémentaires :

```text
motion_test: OK
app_test: OK
window_backend_test: OK
window_lifecycle_test: OK
window_contract_test: OK
```

- `motion_test` couvre la référence initiale, les deux sens de rotation,
  la sensibilité, la pause/reprise, la mort, le redémarrage et les niveaux.
- `app_test` vérifie la configuration, les callbacks, la limitation du pas
  de temps, la progression de l'état et la remontée des erreurs du moteur.
- Le backend de test remplace uniquement la boucle XCB via `LD_PRELOAD`.
  Il utilise une surface Cairo et invoque les vrais callbacks natifs/kc3.
- Les scénarios Échap et fermeture de fenêtre sortent avec zéro ; l'échec
  du backend avant chargement sort avec un. Le message `FAIL_LOAD` est attendu
  dans ce dernier scénario, ce n'est pas un échec de la suite.
- Le contrat natif est testé avec des callbacks absents, non appelables,
  des retours mal formés et `:error`, y compris pendant le nettoyage.
  Des marqueurs de rejet sont donc attendus dans ce journal.
- Compilation stricte et `git diff --check` sans erreur.
- Une revue finale en lecture seule par Sonnet n'a relevé aucun problème
  bloquant dans l'adaptateur, le lanceur, le pont et l'application kc3.
  Cette revue ciblée ne remplace pas les tests ni l'essai interactif.

Le 29 septembre, un exécutable du jeu instrumenté avec AddressSanitizer a
passé les scénarios natifs normal et de contrat. Les bibliothèques du runtime
étaient celles du build habituel. La détection des fuites était désactivée
(`detect_leaks=0`) : LeakSanitizer ne pouvait pas inspecter le processus dans
le sandbox. Il ne s'agit donc pas d'une validation complète des fuites ou du
runtime.

Les journaux fonctionnels sont conservés dans `tests/out/`, notamment
`validation_latest.log`, `app_test.log` et `window_contract_test.log`.

## Mesures et corrections d'intégration

Le benchmark à 30 mobs donne, sur la dernière exécution, 6,633 ms par mise
à jour et 4,061 ms par vue. La référence avant ce lot était respectivement
6,284 et 3,809 ms sur une exécution. Ces mesures isolées ne permettent pas
de conclure à un gain ni d'attribuer l'écart au changement. Elles ne mesurent
pas la boucle graphique complète du nouveau pont.

Trois erreurs ont été corrigées pendant l'intégration : le champ contenant
le jeu devait être typé `%DoomState{}`, les tags temporaires de copie devaient
être initialisés à zéro, et le traitement des arguments devait tenir compte
du fait que `kc3_init` consomme `argv[0]`. Les tests de callbacks et de
chargement de fixture couvrent les chemins qui avaient échoué.

## Limites et suite

Pas de partie interactive réalisée après ce refactor. Lancer `make run`
pour vérifier le confort réel, la fermeture et le redimensionnement.
La souris reste limitée par les bords de la fenêtre ; capture relative et
gestion du focus restent à traiter. Le relâchement des touches, le tir
continu et le son ne font pas partie de ce lot.

Prochain lot pour augmenter la part de kc3 : recettes des textures, puis
HUD. Les évolutions du backend fenêtre nécessiteront leur branche dédiée
et les validations prévues dans `AGENTS.md`.
