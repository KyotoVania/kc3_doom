# Site sur une branche dédiée

À la demande du propriétaire, la publication utilise une branche `gh-pages` isolée
de l'historique du jeu. Elle ne contient que `index.html`, `.nojekyll`, `LICENSE` et
`NOTICE`. La présentation éditable reste sur `master` ; le générateur y valide les
liens avant de les figer sur la révision source. Pas de changement du moteur.

Après les contrôles GCC, Clang et site, le workflow pousse la branche sans force-push,
relit son commit et déploie exactement cet arbre. Les PR n'ont pas accès au job de
publication. Pages utilise le mode Actions afin que le déploiement ne dépende pas
du déclenchement automatique d'un push effectué par `GITHUB_TOKEN`.

## Validation locale avant push

Six tests Python OK, dont la publication sur un dépôt bare temporaire : branche sans
parent initial, quatre fichiers seulement, idempotence, mises à jour avec conservation
de l'historique, état de travail original intact et refus d'un fichier inattendu.
Syntaxe shell, YAML et `git diff --check` : OK.

Suite complète `sh tests/run_all.sh` : code de sortie 0. Sorties des tests historiques :

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
ms/frame (60 updates, 30 mobs en alerte) : 6.51131905
ms/view (60 vues, 30 mobs) : 3.910705
game_test: OK
view_check: OK
```

Compléments : `motion_test`, `app_test`, `window_backend_test`,
`window_lifecycle_test` et `window_contract_test` : OK. `FAIL_LOAD` est le scénario
d'échec attendu. Aucun gain de performance revendiqué. Journal local ignoré :
`tests/out/gh-pages-validation.log`.

Les résultats distants restent consultables dans GitHub Actions. L'intégration
complète kc3 reste testée localement, pas reconstruite par la CI hébergée.
