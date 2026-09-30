## Description

<!-- Que change cette PR, et pourquoi ? -->

## Type de changement

- [ ] Correction de bug
- [ ] Fonctionnalité
- [ ] Optimisation (mesures avant/après jointes obligatoires)
- [ ] Documentation / outillage

## Vérifications

- [ ] J'ai lu `AGENTS.md` et `HANDOFF.md`.
- [ ] `sh tests/run_all.sh` passe en local et sa sortie est jointe ci-dessous.
- [ ] Les six tests historiques restent verts (`engine`, `textures`, `hud`, `data`, `game`, `view_check`).
- [ ] Aucun commentaire ajouté dans le code (sauf en-tête de licence C), aucun commentaire dans les `.kc3`.
- [ ] Tout changement de contrat (`engine.h`, `hud.h`, tuple de vue, Map `hud`) est répercuté sur `src/bridge.c` et `tests/view_check.kc3`.
- [ ] Optimisation : mesures avant/après jointes (`ms/frame` de `game_test` ou micro-bench dédié).
- [ ] Messages de commit : `[TAG] TITRE EN CAPS`, corps en minuscules, sans mention d'outil ou d'IA.
- [ ] Si le runtime kc3 (`../libkc3`, `../window`) est touché : branche dédiée, test red→green, passe ASan — et je sais que le merge nécessite une validation humaine du mainteneur.

## Sortie des tests

```
(coller ici la sortie de sh tests/run_all.sh)
```

## Mesures (si optimisation)

| Mesure | Avant | Après |
|---|---|---|
| `ms/frame` (game_test) | | |
