# Préparer la publication

Le dépôt vise un jeu natif inspiré de Doom en kc3. GitHub Pages est sa vitrine documentaire, pas une version web du jeu.

Dépôt public : [KyotoVania/kc3_doom](https://github.com/KyotoVania/kc3_doom).
Le propriétaire a autorisé sa création et la publication du code. La licence racine
reprend le texte de permission existant, avec les mentions kmx.io et KyotoVania ;
voir [LICENSE](../LICENSE) et [NOTICE](../NOTICE). Les vérifications ci-dessous restent
utiles pour les publications futures.

## Deux branches distinctes

| Branche | Contenu | Modification |
|---|---|---|
| `master` | Jeu, tests, documentation, présentation HTML éditable et outils | Développement normal |
| `gh-pages` | `index.html`, `.nojekyll`, `LICENSE`, `NOTICE` uniquement | Génération automatique après les tests |

`gh-pages` démarre sans parent commun avec `master`, puis conserve son propre historique
de publications. Aucun force-push. Ne pas modifier directement cette branche : éditer
`presentation-code.html` sur `master`. Les liens source sont figés sur le commit du jeu
ayant servi à générer le site. Aucun source du jeu, dump, journal ou binaire n'est copié.

Après les tests, la CI génère le site, met à jour `gh-pages` avec `tools/publish_site.sh`,
puis relit et déploie le commit de cette branche. Le test de publication utilise un dépôt
bare temporaire : création isolée, idempotence, mise à jour, protection de l'index courant
et refus des fichiers inattendus.

## Avant le premier push

- Confirmer le propriétaire, le nom du dépôt public et l'autorisation de commit/push.
- Confirmer avec les ayants droit la licence globale et la provenance des fichiers. Les en-têtes existants sont conservés ; aucune nouvelle licence n'est imposée par cette préparation.
- Vérifier les fichiers à publier **et l'historique Git** : pas de secrets, journaux privés, dumps, exécutables ou configuration machine. `.gitignore` ne retire pas des données déjà commitées.
- Conserver `PLAN.md` et les tâches comme archives techniques, pas comme définition actuelle du projet.
- Exécuter `sh tests/run_all.sh`, les contrôles du site ci-dessous et `git diff --check`.

## CI réellement couverte

Le workflow `ci.yml` s'exécute sur les push, pull requests et lancements manuels :

| Contrôle | Couverture |
|---|---|
| GCC et Clang, Ubuntu 24.04 | Tests autonomes du raycaster, des textures et du HUD ; aucune installation kc3 nécessaire |
| Présentation | Liens locaux, ancres, identifiants dupliqués, tests du générateur, syntaxe JavaScript, génération du site |
| GitHub Pages | Déploiement optionnel après réussite des deux contrôles, uniquement depuis la branche par défaut |

La CI **ne compile pas encore le jeu complet ni le runtime kc3**. La suite `sh tests/run_all.sh` reste une validation locale dans un arbre kc3 construit. Prochaine étape : fixer une révision kc3 reproductible et ses dépendances, puis ajouter cette intégration sur un runner vierge. Aucun résultat de workflow distant n'est revendiqué avant un premier push et une exécution réelle.

Contrôles locaux :

```sh
sh tests/run_native.sh
CC=clang sh tests/run_native.sh
python3 -m unittest discover -s tests -p 'site_test.py' -v
python3 tools/build_site.py --output _site
```

Ouvrir `_site/index.html`. Le répertoire de sortie doit être neuf ; le générateur refuse d'écraser une sortie existante. Pour une seconde prévisualisation, choisir un autre `--output` sous `tests/out/`.

## Activer Pages après création du dépôt

1. Dans **Settings → Pages**, choisir **GitHub Actions** comme source. Le contenu déployé
   provient bien de `gh-pages`, mais le déploiement est explicite : un push avec le jeton
   automatique ne déclenche pas seul un build Pages par branche. Cela évite d'ajouter un
   jeton personnel. Voir la [documentation GitHub](https://docs.github.com/en/pages/getting-started-with-github-pages/configuring-a-publishing-source-for-your-github-pages-site).
2. Dans **Settings → Environments → github-pages**, limiter les déploiements à la branche par défaut.
3. Dans **Settings → Secrets and variables → Actions → Variables**, créer `PUBLISH_PAGES` avec la valeur `true`. Aucun secret personnel n'est nécessaire au workflow.
4. Relancer **CI** depuis la branche par défaut, ou y pousser un changement autorisé.
5. Vérifier l'URL rendue par le job **Publish presentation**, puis l'ajouter au champ Website du dépôt et au README. Ne pas inventer de badge vert avant ce contrôle.

Le site publié contient seulement `index.html`, `.nojekyll`, `LICENSE` et `NOTICE`. Ses liens source pointent vers les fichiers GitHub à la révision exacte du déploiement. Le générateur ne copie ni l'arbre complet, ni la configuration locale, ni le prototype JS dans l'artefact public. En prévisualisation locale, seuls les fichiers source/documentation explicitement liés et les mentions sont copiés.

Le workflow utilise les actions et permissions décrites dans la [documentation officielle GitHub Pages](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages). Les jobs de test sont en lecture seule ; seul le job de publication dispose de `contents: write`, `pages: write` et `id-token: write`. Les pushes sur `gh-pages` sont exclus de la CI pour éviter une boucle. Dependabot propose mensuellement les mises à jour des actions.

En cas d'échec, relancer CI sur le dernier commit de `master`. Si l'arbre généré est
identique, aucun commit supplémentaire n'est créé. Désactiver `PUBLISH_PAGES` suspend
les publications suivantes, sans supprimer le site existant ni son historique.

## Présentation GitHub proposée

- Description : « Un jeu natif inspiré de Doom, avec logique et orchestration en kc3, rendu Cairo/C et migration progressive vers kc3. »
- Topics : `kc3`, `game-development`, `raycasting`, `cairo`, `c`, `experimental`.
- Protéger la branche par défaut et exiger les contrôles **Native tests (gcc)**, **Native tests (clang)** et **Presentation checks** avant fusion, une fois leur premier passage confirmé.
- Le nom Doom reste une référence d'inspiration : ne pas laisser entendre une affiliation, un port officiel ou une compatibilité avec les fichiers du jeu original.
