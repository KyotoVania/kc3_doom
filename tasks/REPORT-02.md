# Rapport fiche 02 : textures procédurales en cairo

## Fichiers créés

- `src/textures.c` : `textures_init` + `textures_rnd_test` (seule fonction publique
  supplémentaire). Portage du RNG mulberry32 (graine 7) et des 47 textures de
  `kmx_doom.html` lignes 23-115. Ne dépend que de cairo, libm et `engine.h`.
- `tests/textures_test.c` : test autonome (RNG, pixels, atlas PNG).
- `.gitignore` contenait déjà `tests/out/` : pas de modification nécessaire.

## Commande de test

```sh
mkdir -p tests/out
cc -std=c11 -W -Wall -Werror -pedantic -O2 $(pkg-config --cflags cairo) \
   -Isrc src/textures.c tests/textures_test.c -o tests/out/textures_test \
   $(pkg-config --libs cairo) -lm
./tests/out/textures_test
```

Sortie : `textures_test: OK`, aucun warning.

Le test vérifie les 5 premiers tirages et le 1001e du RNG à 1e-12 près
(valeurs de référence de la fiche), que chaque texture a au moins un pixel
d'alpha ≥ 128, qu'aucun canal n'est hors limites, et écrit
`tests/out/textures_atlas.png` (8 colonnes × 6 lignes, textures ×2, fond
damier gris).

## Description de l'atlas, texture par texture

Ligne 1 (murs) :
- `TEX_WALL_1` : panneau bleu nuit à deux bandes, boulons gris, `kmx` orange,
  `.io` bleu clair. Conforme.
- `TEX_WALL_2` : fond noir-vert, 9 lignes de code kc3 vertes (dont certaines
  jaunes), cadre vert sombre. Le choix des lignes suit les tirages `rnd` : même
  tirage que le JS.
- `TEX_WALL_3` : blason doré moucheté, 12 rayons, poisson-globe jaune avec œil,
  `OpenBSD` en bas. Conforme.
- `TEX_WALL_4` : rack `OVH`, 7 serveurs gris avec ventilations, LED vertes et
  orange tirées par `rnd`, label en bas. Conforme.
- `TEX_WALL_5` : briques rouges, bandeau noir translucide, `SIGSEGV` jaune,
  `core dumped` blanc. Conforme.
- `TEX_WALL_6` : briques grises. Conforme.
- `TEX_WALL_D` : porte grise à deux battants, bandes jaune/noir en haut et en
  bas, `[E]` jaune au centre. Conforme.
- `TEX_WALL_X` : terminal noir `$ git push` / `origin` / `master` en vert,
  curseur, `EXIT [E]` rouge. Conforme.

Ligne 2 (sols + segv) :
- `TEX_FLOOR_TILE` : damier 2×2 gris foncé, cadre, grain. Conforme.
- `TEX_FLOOR_GRATE` : grille métallique croisée sur fond sombre. Conforme.
- `TEX_FLOOR_CARPET` : moquette bleu nuit mouchetée, liseré orange. Conforme.
- `TEX_FLOOR_PANEL` : panneau gris, bords sombres haut/gauche, carré jaune
  pâle central. Conforme.
- `TEX_FLOOR_DARK` : deux carrés gris en diagonale, point rouge central.
  Conforme.
- `TEX_SEGV_WALK_0/1`, `TEX_SEGV_ATK` : octogone rouge à liséré blanc, jambes
  noires écartées en walk 1, yeux et sourcils froncés, `SEGV`, bouche jaune en
  attaque. Conforme.

Ligne 3 :
- `TEX_SEGV_DEAD` : flaque rouge sombre `0xe8`. Conforme.
- `TEX_LEAK_WALK_0/1`, `TEX_LEAK_ATK` : goutte d'eau turquoise (béziers),
  reflet translucide, gouttes au sol variables, bouche ouverte en attaque
  (teinte plus claire). Conforme.
- `TEX_LEAK_DEAD` : flaque turquoise `free()`. Conforme.
- `TEX_RACE_WALK_0/1` : deux fantômes violets décalés, le premier
  semi-transparent, franges alternées. Conforme.
- `TEX_RACE_ATK` : fantôme de droite violet clair. Conforme.

Ligne 4 :
- `TEX_RACE_DEAD` : flaque violette `mutex`. Conforme.
- `TEX_DFREE_WALK_0/1`, `TEX_DFREE_ATK` : deux boules orange jambes noires,
  `free()` sous chacune, décalage vertical alterné en walk 1, teinte claire en
  attaque. Conforme.
- `TEX_DFREE_DEAD` : deux flaques orange. Conforme.
- `TEX_FREE1_WALK_0/1`, `TEX_FREE1_ATK` : grosse boule orange `free()`.
  Conforme.

Ligne 5 :
- `TEX_FREE1_DEAD` : flaque orange `NULL`. Conforme.
- `TEX_BOSS_WALK_0/1`, `TEX_BOSS_ATK` : masse bordeaux à 8 tentacules
  (quadratiques), gros œil blanc à pupille rouge en attaque, 7 dents
  descendantes, `env->err` jaune au-dessus. Conforme.
- `TEX_BOSS_DEAD` : flaque bordeaux `err: locked`. Conforme.
- `TEX_THOMAS_0` : bonhomme torse noir `kmx`, bras baissés, tête ronde,
  cheveux, yeux et bouche. Conforme.
- `TEX_THOMAS_1` : bras droit levé (main en haut). Conforme.
- `TEX_ITEM_COFFEE` : tasse blanche `kmx`, anse à droite, vapeur grise,
  bandeau café. Conforme.

Ligne 6 :
- `TEX_ITEM_AMMO` : caisse verte `BRK` jaune, cadre sombre. Conforme.
- `TEX_ITEM_FACT` : halo cyan translucide, `{ }` et `fact`. Conforme.
- `TEX_ITEM_GDB` : pince/étau gris et marron, `gdb` jaune. Conforme.
- `TEX_ITEM_ASAN` : barrette grise, encoches, zone rouge, `ASan`. Conforme.
- `TEX_ITEM_ARMOR` : bouclier vert, coche claire, `tests`. Conforme.
- `TEX_PROJ` : dégradé radial blanc → orange → rouge transparent, `0x0`.
  Conforme.
- `TEX_BPROJ` : dégradé radial blanc → rose → violet transparent, `err`.
  Conforme.

## Écarts avec le JS

- Police : `cairo_select_font_face("monospace", ...)` (toy font API) n'est pas
  la police du navigateur ; tailles en px reprises telles quelles. Les labels
  restent lisibles et aux mêmes positions (alignement via `x_advance`, baseline
  middle via `y_bearing + height / 2`).
- Antialiasing et rasterisation cairo ≠ Canvas 2D : contours des arcs, ellipses
  et béziers diffèrent d'un pixel par endroits.
- Les valeurs dé-prémultipliées suivent `min(255, c * 255 / a)` en entier, le
  JS stocke des octets issus du canvas (arrondi navigateur) : écarts de ±1
  possibles sur les pixels semi-transparents (fantôme race, halos, flaques).
- `textures.c` est autonome : `engine_init` non requis, seul `engine->tex` est
  écrit. Rien d'autre n'a été modifié (`engine.h`, `kmx_doom.c`, `../kc3`
  intacts).

## Points non faits

Aucun.
