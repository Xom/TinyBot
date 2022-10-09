== INPUT FEATURES ==

Global features (11, each scaled to interval [0, 1]):
0. Ink - 0 if not drawing; 1 before initial draw; 2 after initial draw; max 12
1. Forests in deck - max 13
2. Houses in deck - max 10
3. Sands in deck - max 9
4. Waves in deck - max 9
5. Mountains in deck - max 5
6. Churches in deck - max 5
7. Boats in deck - max 3
8. Placements remaining - max 26
9. Placements until draw - max 9
10. Drawings completed - max 2

Local features (21x9x9, scaled to interval [0, 1]):
0. Land
1-7. Places
8. Drawable - includes special case for surround AKA indirect; 0 if impossible; 0.25 if possible later; 1 if indirect; else priority in (0.5, 1)
9-15. Placeable
16. Columns in deck - max 2
17. Rows in deck
18. Nonants in deck
19. Wave status of column - ignores land status; 0 if no wave; 1 if valid wave; 2 if invalid wave
20. Wave status of row

== OUTPUT FEATURES ==

Policy (flatten(8x9x9)+1 i.e. 649):
0* Draw
1-7* Place
648. Pass

Score (8, scaled 1/128, only used in training):
0. Mean penalty, absolute value
1-7. Mean score per place type

Land (1x9x9, only used in training)
