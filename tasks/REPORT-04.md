# Rapport fiche 04 : HUD, arme, visage et écrans en cairo

## Fichiers créés

- `src/hud.h` : contrat `s_hud` exact de la fiche, `hud_draw`.
- `src/hud.c` : tout le dessin 2D (arme, réticule, flashs, bulle Thomas,
  messages, minimap, barre HUD, visage, écrans titre/pause/mort/inter/win).
  Ne dépend que de cairo, libm, libc, `engine.h` et `hud.h`.
- `tests/hud_test.c` : programme autonome, 7 PNG dans `tests/out/`.
- `.gitignore` : contenait déjà `tests/out/`, inchangé.

## Test

```
$ mkdir -p tests/out
$ cc -std=c11 -W -Wall -Werror -pedantic -O2 $(pkg-config --cflags cairo) \
     -Isrc src/hud.c tests/hud_test.c -o tests/out/hud_test \
     $(pkg-config --libs cairo) -lm
$ ./tests/out/hud_test
hud_test: wrote tests/out/hud_title.png
hud_test: wrote tests/out/hud_play_printf.png
hud_test: wrote tests/out/hud_play_gdb_hurt.png
hud_test: wrote tests/out/hud_play_asan_intro.png
hud_test: wrote tests/out/hud_dead.png
hud_test: wrote tests/out/hud_inter.png
hud_test: wrote tests/out/hud_win.png
hud_test: OK
```

Compilation sans warning (`-W -Wall -Werror -pedantic`).

## Description des PNG (comparaison avec le JS)

Fond gris de test partout (chez le vrai appelant, les 200 premières lignes
contiennent la vue 3D).

1. `hud_title.png` : panneau noir à 72 %, « KMX DOOM » 46 px gras avec
   ombre `#400` décalée de 3 px (l. 461), « — LE STAGE — » jaune,
   sous-titre `· kmx.io · 2026`, les 4 lignes de contrôles à y=138+13i,
   et « CLIQUE POUR COMMENCER » visible (blink actif : time=0.6).
   Aucun élément de jeu dessiné, comme en JS quand state='title'.
2. `hud_play_printf.png` : bulle Thomas en haut (fond `#0a0c12` à 85 %,
   barre orange, « THOMAS DE GRIVEL · kmx.io » et la ligne), les 2
   messages jaunes avec ombre noire en dessous (my=46 puis 57),
   minimap en haut à droite (fond noir 60 %, murs gris `#8a8f98`,
   portes jaunes `#e8b820`, sortie verte `#3f6`, 5 mobs rouges, Thomas
   orange, joueur blanc avec trait de direction), réticule blanc 70 %
   au centre, arme printf : étoile de tir à 12 branches, trapèzes
   `#2c2e33`/`#484b52`, label vertical « printf », main `#d9a877`,
   manche `#1d2a44`. Barre HUD : BREAKPOINTS 120, SANTÉ 100%, touches
   « 1 2 3 » (1 jaune, 2 clair, 3 grisé), nom « printf », visage,
   TESTS 50%, BUGS 7/12, FACTS 2/4, nom de niveau.
3. `hud_play_gdb_hurt.png` : voile rouge `rgba(255,0,0,0.24)` sur la
   vue (hurt_t=0.2), fusil gdb à deux canons avec label « gdb », visage
   bouche ouverte `#300`, barre noire au-dessus des yeux (hp<50) et
   4 traces de sang (n=(80-40)/10), SANTÉ 40%.
4. `hud_play_asan_intro.png` : texte d'intro « E1M1 : libkc3/ » rouge
   22 px et « Le runtime » blanc au centre (alpha=min(1,intro)=1),
   mitrailleuse ASan : corps `#50545c`, 4 barillets `#24262a`,
   bande rouge `#cc2222`, label « ASan », étoile de tir de rayon
   16+rand()*6, « 1 2 3 » toutes possédées avec 3 en jaune.
5. `hud_dead.png` : voile rouge `rgba(120,0,0,.6)` sur tout l'écran,
   « Segmentation fault » blanc 22 px, « (core dumped) » `#fcc`,
   « clique pour relancer le niveau » visible (state_t=1 > 0.8),
   visage mort (yeux « x ») sous le voile, SANTÉ 0%.
6. `hud_inter.png` : panneau, « NIVEAU TERMINÉ » rouge, nom du niveau,
   stats « Bugs fixés 83% », « Facts 75% », « Temps 1:23 » (labels à
   gauche x=120, valeurs jaunes alignées à droite x=280), ligne Thomas
   avec l'outro, « clique pour continuer » visible.
7. `hud_win.png` : « STAGE VALIDÉ » vert 30 px, ligne Thomas avec le
   texte JS par défaut, stats globales « Bugs fixés 30/34 »,
   « Facts 9/12 », « Temps total 12:34 », « Merci Thomas, merci
   kmx.io. », « Reste plus qu'à écrire le rapport de stage... »,
   « clique pour revenir au titre » visible (state_t=2 > 1.2).

## Correspondances et écarts avec le JS

- Ordre de la ligne 542 respecté : si `state != HUD_TITLE`,
  `drawPlayOverlay` puis `drawHUD`, puis `drawOverlay` dans tous les cas.
- `txt`/`shadowTxt` : centrage par `cairo_text_extents`
  (x − width/2 − x_bearing), baseline « middle » par
  y − (y_bearing + height/2), ombre noire décalée de (1,1). Alignements
  center/left/right portés. La police est celle de fontconfig
  (monospace), pas celle d'un navigateur : métriques légèrement
  différentes, positions identiques.
- `Impact` : `cairo_select_font_face("Impact", …, BOLD)`, retombe sur
  la police gras par défaut ici (Impact non installée). Aucune police
  embarquée.
- `globalAlpha` : alpha multiplié dans la source de chaque primitive
  (messages, intro), rendu équivalent.
- `Math.random` (flash ASan) : `rand()` de la libc, cosmétique.
- `drawFace` : branche sans image `FACE` uniquement (l. 406-416).
- `drawMap` : grille lue via `engine_grid_get`, `s = min(4, 130/cols)`,
  `D` jaune, `X` vert, autres murs gris, `.` ignoré. Le trait de
  direction est tracé avec `cairo_set_line_width(cr, 1.0)` car le
  défaut cairo est 2.0 contre 1.0 pour canvas.
- `#c22`, `#3f6` etc. : couleurs CSS courte étendues (`#c22` →
  `0xcc2222`).
- Écart volontaire : l'écran win du JS code en dur
  « Commit validé. Une ligne, sans corps. » ; ici `hud->outro` est
  utilisé si renseigné, sinon ce texte par défaut (le test laisse
  `outro` NULL sur ce cas).
- Écran win du JS : stats globales en « n/m » (pas en %), porté tel
  quel ; `pct` affiche « — » si le total est 0.
- `hud_concat` (préfixe « Thomas : ») est une concaténation bornée à
  la main : `strnlen` n'est pas C11 strict.

## Test

`tests/hud_test.c` ne linke pas `engine.c` : il fournit sa propre
définition de `engine_grid_get`, marquée comme stub de test par la
variable globale `g_hud_test_engine_grid_get_stub` en tête de fichier.
La grille 24×19 (murs `1`/`2`, trois portes `D`, une sortie `X`) est
remplie directement dans une `s_engine` statique, avec vérification de
la longueur des lignes au démarrage.

## Points non faits

Aucun. Le remplissage de `s_hud` depuis kc3 relève de la fiche 05.
