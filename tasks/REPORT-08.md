# Lot 08 — préparation du dépôt public (30/09/2026)

## Livré localement

- README, guide de contribution et modèles d'issues/PR rédigés par Kimi K3 puis relus et ajustés par le coordinateur.
- Positionnement corrigé dans le README, la présentation et le handoff : refaire un jeu inspiré de Doom en kc3 ; le prototype JavaScript est une référence historique, pas la finalité. Le plan initial est explicitement archivé.
- Workflow GitHub Actions : tests C autonomes GCC/Clang, contrôles et génération du site, déploiement Pages optionnel après réussite des tests.
- Générateur sans dépendance Python externe : valide les liens et ancres, publie seulement la présentation, transforme les liens source en liens GitHub à la révision exacte. Prévisualisation locale également disponible.
- Dependabot pour les actions et procédure de publication dans `.github/PUBLISHING.md`.

Pas de changement du gameplay ni du runtime dans ce lot. Les modifications de jeu déjà présentes dans l'arbre sont conservées. Aucun commit, remote, dépôt GitHub, push ou déploiement n'a été créé.

## Validation locale

`sh tests/run_native.sh` et `CC=clang sh tests/run_native.sh` : succès tous deux.

Sortie des six tests historiques dans `sh tests/run_all.sh` :

```text
engine_test: OK
textures_test: OK
hud_test: wrote tests/out/hud_title.png
hud_test: wrote tests/out/hud_play_printf.png
hud_test: wrote tests/out/hud_play_gdb_hurt.png
hud_test: wrote tests/out/hud_play_asan_intro.png
hud_test: wrote tests/out/hud_dead.png
hud_test: wrote tests/out/hud_inter.png
hud_test: wrote tests/out/hud_win.png
hud_test: OK
data_test: OK
ms/frame (60 updates, 30 mobs en alerte) : 8.640109133333334
ms/view (60 vues, 30 mobs) : 5.021088000000001
game_test: OK
view_check: OK
```

Suite complémentaire :

```text
motion_test: OK
app_test: OK
window_backend_test: OK
window_backend_test: WM_CLOSE
window_backend_test: FAIL_LOAD
DoomApp.main -> false
window_lifecycle_test: OK
window_contract_test: OK
```

`FAIL_LOAD` et `DoomApp.main -> false` sont attendus dans le scénario d'échec de chargement. Le lanceur complet termine avec le code 0. Les mesures ci-dessus sont une observation de cette exécution, sans comparaison avant/après ni revendication d'optimisation. Journal brut local, ignoré par Git : `tests/out/public-repo-validation.log`.

- `python3 -m unittest discover -s tests -p 'site_test.py' -v` : **5 tests OK**, y compris cas négatifs et contenu exact de l'artefact public.
- Génération locale : **21 fichiers liés vérifiés**, prévisualisation dans `tests/out/public-site/index.html`.
- JavaScript inline : `node --check`, succès.
- Syntaxe YAML des quatre fichiers GitHub : chargement PyYAML, succès. Ce contrôle ne remplace pas une exécution GitHub Actions.
- `sh -n tests/run_native.sh` et `git diff --check` : succès.

## Avant publication

Confirmer le propriétaire/nom, la licence et l'autorisation de commit/push ; auditer les fichiers et l'historique destinés au public. La connexion GitHub est disponible mais aucun remote du projet n'est configuré. Après publication : activer Pages et `PUBLISH_PAGES=true`, puis vérifier une première exécution distante avant d'annoncer la CI verte ou une URL publiée.

La CI ne couvre pas encore l'intégration complète avec un runtime kc3 reconstruit sur un runner vierge. Une révision amont et une procédure de build reproductible restent à fixer. Aucune validation interactive supplémentaire n'a été effectuée dans ce lot.

## Suivi — premier envoi public autorisé

Le propriétaire a ensuite autorisé la création de `KyotoVania/kc3_doom`, le commit et
le push avec mentions kmx.io et KyotoVania. Le dépôt public a été créé avant l'envoi.
`LICENSE` reprend le texte de permission existant ; `NOTICE` précise sa portée, et
les crédits sont présents dans le README, les en-têtes C existants et les pages HTML.
La nouvelle priorité EGL/rendu GPU est décrite dans `09-rendu-egl.md`, sans changement
de backend dans cette publication.

Validation répétée avant commit :

```text
engine_test: OK
textures_test: OK
hud_test: wrote tests/out/hud_title.png
hud_test: wrote tests/out/hud_play_printf.png
hud_test: wrote tests/out/hud_play_gdb_hurt.png
hud_test: wrote tests/out/hud_play_asan_intro.png
hud_test: wrote tests/out/hud_dead.png
hud_test: wrote tests/out/hud_inter.png
hud_test: wrote tests/out/hud_win.png
hud_test: OK
data_test: OK
ms/frame (60 updates, 30 mobs en alerte) : 6.31541235
ms/view (60 vues, 30 mobs) : 3.8585239833333334
game_test: OK
view_check: OK
motion_test: OK
app_test: OK
window_backend_test: OK
window_lifecycle_test: OK
window_contract_test: OK
```

Suite complète : code de sortie 0. Contrôles du site : 5 tests OK, 24 fichiers liés
vérifiés, dont LICENSE, NOTICE et le lot EGL. Aucun gain de performance revendiqué.
Journal brut local ignoré : `tests/out/publication-validation.log`.
Une recherche ciblée de formats de secrets dans les fichiers publiables et les blobs
de l'historique n'a trouvé aucun motif surveillé ; ce n'est pas un audit de sécurité
exhaustif. Configurations locales, exécutables, objets, caches et journaux restent exclus.
