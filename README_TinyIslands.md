# Appendix: What is Tiny Islands?

[Tiny Islands](https://dr-d-king.itch.io/tiny-islands) is a solitaire game by [David King](https://dr-d-king.itch.io/) played on a 9x9 board on which the player places tiles and draws shorelines, resulting in a final score.

The easiest way to become familiar with the game is to try it for a few minutes. But for reference, here follows a formal description.

There is a 54-card deck of tiles and a 54-card deck of zones, shuffled at the start.

The tile deck contains:
* 13 Forest
* 10 House
* 9 Sand
* 9 Wave
* 5 Mountain
* 5 Church
* 3 Boat

A zone is a row, column, or nonant on the board; each zone contains 9 cells. A nonant is the 3x3 intersection of the 3 top, middle, or bottom rows and the 3 left, middle, or right columns. In TinyBot's game records, rows are labeled 1-9 from bottom to top, columns a-i from left to right, and nonants QWE ASD ZXC according to QWERTY positions. The zone deck contains 2 of each zone.

Each turn, two pairings of tile and zone are drawn from the decks. The player must place the first tile on an empty cell in the first zone or the second tile in an empty cell in the second zone. In the rare case that both zones (possibly both instances of the same zone card) are full, the game cannot be continued and there is no score; in TinyBot's evaluation, this state has a large hardcoded penalty, so that the bot avoids it. The game ends after 26 turns, that is, after placing 26 tiles, having drawn 52 of the 54 cards in each deck.

At the end of the 9th, 18th, and 26th turns, the player draws a shoreline along the edges of cells, which must be a closed loop, not crossing or touching itself or any other shoreline; that is, at each intersection of cell edges, exactly zero or two of the edges connected to that intersection can be part of a shoreline. Shorelines must comprise fewer than 25 edges; combined with the fact that shorelines are closed loops, this means shorelines contain an even number of edges between 4 and 24, inclusive. Shorelines can be wholly inside or outside other shorelines.

At the end of the game, each cell with zero or two shorelines between it and the outside of the board is water, and with one or three, land. Sand, Wave, and Boat are valid when on water, and the other tiles when on land. Invalid tiles score -5 points and are ignored for the rest of scoring.

The final score is the sum of the scores of each tile on the board. In the following rules for scoring (valid) tiles, 'near' refers to a cell's 8 neighbors, orthogonal and diagonal.
* Each orthogonally contiguous group of Forest scores 2 points for each Forest in it, minus a 2-point "group tax".
* House scores 1 point for each unique non-House tile near it.
* Sand scores 1 point for each of its cell's edges that's part of a shoreline.
* Wave scores 2 points unless it's near another Wave, or in the same row or column as another Wave.
* Mountain scores 2 points for each Forest near it.
* Church scores 0 points if on the same contiguous landmass as another Church, else 2 points for each House near it plus 1 point for each additional House elsewhere on the same contiguous landmass.
* Boat scores 1pt for each orthogonal step between it and the closest land or other Boat. (An orthogonal neighbor would be one step; a diagonal neighbor, two. But the closest may not be a neighbor at all.)

[A strategy guide.](https://imgur.com/a/yMPK279)

TinyBot's objective is to maximize its mean expected score, with no foreknowledge of the shuffle.

## Addendum: the pattern in the deck

Contrary to the above, there is actually only one 54-card deck, with tiles on one side and zones on the reverse side. For each placement turn, the previous turn's tiles are flipped over to reveal the zones on the reverse side. Each game, the tiles on the other side of the first two zones and the zones on the other side of the last two tiles are unseen and unused.

For example, the three Boats in the deck have the topmost row, the leftmost column, and the bottom left nonant on their backs. This results in lesser Boat values than the theoretical two-deck scheme previously described, because it results in 4% fewer opportunities to play Boats in corners.

When I started developing TinyBot, I read the developer's comment describing this, but when I scrutinized the digital implementation, I mistakenly determined it not to be the case. I didn't discover my mistake until after TinyBot was done training. So TinyBoat was trained using the "naive" two-deck scheme, and is not aware of the actual deck pattern. I could train TinyBot further to be aware, but I wish to move on from the project, so I won't.

For the final test run, I generated 4032 "naive" shuffles not using the actual deck pattern, and I also had 62 shuffles transcribed from the actual game, containing the actual deck pattern. TinyBot had a mean score of 72 on the naive shuffles, and 70.7 on the transcribed shuffles. Thanks to the large sample of naive shuffles, this difference is statistically significant. How much of it is due to the effect on Boats I described, versus how much of it is due to other (similar) effects, I don't know.

For what it's worth, my speculation is that actually being aware of the pattern wouldn't be useful until the last couple of turns, because knowing the next zones doesn't seem that useful without knowing the next tiles. On the last couple of turns, though, you'd have a better idea of which tiles are on the other side of the first two zones and won't be used, giving you a good guess at the last few tiles before you see them.
