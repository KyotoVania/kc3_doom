# REPORT-01 : moteur de rendu C (raycaster)

## Fichiers créés

- `src/engine.c` : implémentation complète de `src/engine.h` (sauf
  `textures_init`, fiche 02).
- `tests/engine_test.c` : test autonome (C pur + cairo).
- `.gitignore` : contenait déjà `tests/out/`, rien à ajouter.

## Commandes lancées

```sh
mkdir -p tests/out
cc -std=c11 -W -Wall -Werror -pedantic -O2 $(pkg-config --cflags cairo) \
   -Isrc src/engine.c tests/engine_test.c -o tests/out/engine_test \
   $(pkg-config --libs cairo) -lm
./tests/out/engine_test
```

Sortie : `engine_test: OK` (aucun warning à la compilation).

Le test vérifie :

- `engine_los` sur 5 cas : même pièce → true, à travers un mur → false,
  points confondus → true, direction axiale (dx = 0) → true sans boucle
  infinie, axial à travers un mur → false ;
- `engine_cast` depuis (1.5, 1.5) vers +x : touche la cellule (5, 1),
  `side == 0`, distance 3.5 ± 1e-9 ;
- rendu d'une frame depuis P (1.5, 1.5), pa = 0.3, 3 sprites, frame non
  noire, `zbuf` strictement positif sur toutes les colonnes ;
- écriture de `tests/out/engine_e1m1.png` (ARGB32 400×250 : frame +
  fond noir) via `engine_blit` + `cairo_surface_write_to_png`.

## Image produite

`tests/out/engine_e1m1.png` montre un couloir E1M1 en perspective :
murs dégradés (textures de test déterministes `id*37 / x*4 / y*4`),
faces `side == 1` assombries (facteur 0.78), sol et plafond texturés
avec le plafond plus sombre (`f * 0.8`), brouillard vers le fond. Un
gros disque opaque au centre (sprite SEGV), un petit disque éclairci à
droite (sprite COFFEE avec `flash` → `bright()`), bords des sprites en
alpha 0 transparents. Le troisième sprite (PROJ en (7.5, 1.5), derrière
le mur `x = 5` de la ligne 1) n'apparaît pas : masqué par `zbuf`. Les
50 lignes du bas sont noires (zone HUD hors frame).

## Écarts avec le JS

- `buf.fill(0xff000000)` devient `memset(frame, 0, ...)` : la frame est
  en 0x00RRGGBB (alpha à 0) comme exigé par le contrat ; `shade` et
  `bright` ne remettent pas le canal alpha à 0xff.
- `Math.abs(1/0)` (Infinity) est remplacé par `1e30` pour les `delta`
  du DDA quand `dx` ou `dy` vaut 0, comme demandé.
- Pas de `Math.random` : le clignotement des `race` (ligne 282) n'est
  pas porté, il revient à l'appelant de ne pas ajouter le sprite.
- Tri des sprites par distance décroissante : tri insertion stable sur
  un tableau d'ordre en pile (pas de `qsort`, aucune allocation
  dynamique dans `engine_render`).
- `solid()` du JS (`c !== '.'`) est remplacé par l'appartenance à
  `123456DX` : tout autre caractère est traité comme vide (défense).

## Points non faits

- `textures_init` : fiche 02 (déclarée dans `engine.h`, non définie ici,
  le test ne l'appelle pas).
