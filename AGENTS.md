# AGENTS.md (kc3_doom)

Lire **`HANDOFF.md` en entier avant toute action.** Il décrit l'état du jeu, l'architecture, les contrats, les mesures, les pièges kc3 et les pistes (section 8).

Règles :
- Aucun commentaire dans le code, sauf l'en-tête de licence des fichiers C. Pas de commentaires du tout dans les `.kc3`.
- `libkc3/kc3.h` toujours inclus en premier dans un `.c`.
- Les 6 tests de `HANDOFF.md` section 4 doivent rester verts. Joindre leur sortie au compte rendu.
- Toute optimisation se prouve par une mesure avant/après (`ms/frame` de `tests/game_test.kc3` ou micro-bench dédié). Pas de gain supposé.
- Modifications du runtime kc3 (`../libkc3`, `../window`) : sur une branche kc3 dédiée, jamais sur `master`, avec un test red→green et une passe ASan. Ne rien merger sans validation humaine.
- Ne pas commiter sans accord explicite. Commits `[TAG] TITRE EN CAPS` + corps en minuscules, sans aucune mention d'IA.
- Ne jamais lancer `kc3s` depuis un dossier qui contient un `kc3.dump`.
