**2022-10-22**

I'm writing this chronological account of TinyBot's development in exhausting, and maybe boring, detail, before I forget too many of those details, because having discovered a blind spot in the bot's policy, I'm taking some time to try a remedy, instead of stopping training soon as I'd originally intended, since the score improvement had slowed to a crawl in the interval of 70-71.

I still intend a separate, more reader-friendly writeup once the project has truly concluded.

For reference, [README_TinyIslands.md](README_TinyIslands.md) describes the game.
[README_net.txt](README_net.txt) describes the neural net inputs and outputs.

**First try in Java, two years ago (2020)**

Two years ago, I wanted to learn the Monte Carlo Tree Search (MCTS) algorithm by trying to implement it, with a view to following up with the rest of the "expert iteration" machine learning scheme used in AlphaGo and KataGo. I chose Tiny Islands because it seemed ML-friendly in gamestate representation and in game mechanics, and also because my friends were playing it and I wanted to see if we could learn anything interesting from observing the bot. I used Java, the language I know best.

In the tree search, I handled the randomness by generating 2-4 outcomes for each random event, and discarding the tree when the bot decided on a move and encountered the actual random outcome. Later, when I decided to shelve the project, I thought there wasn't much point in doing tree search past the random events in Tiny Islands, and I should just do a rollout to the end, and not collect stats on any particular random outcome. This is indeed how the current (C++) bot works. In effect, the tile placements become not much of a tree search, having only root and leaf nodes with no intermediates.

At first I represented drawing each shoreline segment as a separate turn, but the bot had trouble completing any legal shorelines enclosing more than one or two cells. So then instead I had the bot choose one cell at a time, subject to the set of chosen cells being contiguous, with the option to pass after at least one cell was chosen.

This worked better, resulting in a mean score of 38. The bot still strongly preferred water tiles, because they're easier to avoid penalties with, as seen in this game that happened to match the mean score of 38:
![38 points with islands of 2, 3, and 7 cells; using 9 land tiles and 17 water tiles](https://cdn.discordapp.com/attachments/218837764701945858/763222612243120128/unknown.png)

Then I hardcoded the bot to choose penalty-affecting cells immediately whenever possible, and the mean improved to 42.

Next, I found the library DeepLearning4Java, implemented and trained a 6-block, 96-channel neural net based on KataGo, and used it to bias the bot's policy. (I don't remember thinking about using the value head.) This caused the rollouts to be much better at avoiding penalties, and the bot no longer strongly preferred water tiles. The mean score was 50-60 points, depending on the number of rollouts allowed.

But it was too slow. Far too slow to generate enough game records for another iteration. I investigated and found that it was spending most of its time copying data to and from GPU RAM. It didn't seem right, but after thinking a little bit and not coming up with a fix, I decided to shelve the whole project until I felt like learning C++. (Despite that presumably someone out there is using DL4J without problems.)

**Interlude: on the choice of algorithm**

I had read up on transposition tables to see how hard it would be to implement, but in the end I came up with a different idea, for avoiding transpositions altogether. I had the idea around the time I shelved the Java project, and implemented it in the current (C++) bot.

I represented what is essentially a subset-selection (or set-partition) task by designating a fixed sort ordering for the elements (which are always the same in my game: the 81 cells of a 9x9 board), and asking the bot to choose one at a time until it wants to pass, and only allowing elements to be chosen in sort order. This stipulation eliminates transpositions when I do tree search. I'd love to know if this is a common way to do subset selection.

In Tiny Islands, I decide the sort ordering after the first cell is chosen, and an approximate description of it would be "by distance to the edge nearest the first cell, then to edge next nearest"; since some cells only become available after cells later in the ordering are chosen, I allow those to be chosen as long as they've never been passed up in favor of some later cell since becoming available. All shorelines not containing other shorelines are possible this way. I have some special cases to allow drawing shorelines containing other shorelines, though they're not exhaustive in allowing all such to be drawn.

At some point in the two intervening years, I also realized that the shoreline drawing problem is not naturally a problem to be using tree search on, but rather a good subject for simulated annealing. Except I have no ready example of neural-network-guided simulated annealing to copy, so I decided to stick with tree search. (Also, my original goal was to learn MCTS, with machine learning as an optional continuation.)

Also, the entire scheme of expert iteration is probably overkill for Tiny Islands, and probably some less complicated Q-learning approach would be sufficient. But at the time, I found expert iteration easier to understand than Q-learning.

Another unusual choice I made is that I coded the tree search to decide on a move once it passed some threshold of visits. This seemed more sensible to me than the usual approach of deciding on a move after some total number of visits are made among all possible moves, because it would spend more time considering more difficult choices, and less time on easy ones. I asked an academic about my idea, who replied noncomittally, speculating that the usual approach might be usual because it made benchmarking easier, either for parameter tuning or for reporting results.

**C++**

Two years later, I thought it would be a good idea to finish the project before starting a new job search, so I set to the task of learning C++, or at least enough of it to implement TinyBot. I implemented the tree search, with the differences described earlier. This time, after some experimentation, I got the mean score to -4, which, though much less than 38 or 42, presumably due to fewer rollouts (i.e. less thinking), is enough better than random play that I decided to move on to the neural net.

Once again, I implemented a 6-block, 96-channel neural net based on KataGo, this time in Keras, using Python. But this model suffered exploding gradients in training, no matter how I tweaked it. I took a snapshot of the net from before the gradients exploded, and turned to coding the search to use it.

I tried using the value head output in place of full rollouts, the way AlphaGo and KataGo do, but it didn't work well. Presumably I needed training data on the results of bad moves, otherwise the resulting positions would be out-of-distribution, but that would be much more complicated to code. I decided to stick to using only the policy head output, even though using the value head could be a 10x-50x speedup, because I didn't want to get bogged down trying to get the training data right. Speaking now that the bot has taken a couple months to be very close to its probable final strength, I'm happy with my decision.

With an amortized\* thinking time of approx. 6 seconds, the result of incorporating this net was a mean score of approx. 46.\*\* (\*The bot plays many games in parallel in order to do GPU work in bigger batches which are more time-efficient. \*\*In order to explore more variety of positions, training games start with 0-7 random moves. But starting with this 46, all mean scores I mention do not include games with 1 or more random starting moves. Based on a quick-and-dirty regression on more recent games, with a mean score of approx. 70 when no random starting moves, the impact of n random starting moves (not more than 7, of course) seems to be approx. -0.83n - 0.06n^2; i.e. the first random starting move decreases the score by about 0.9, and it gets more severe from there.) This C++ implementation is hundreds of times faster than when I had the problem with Java and DL4J.

I abandoned my Keras implementation, and reused the new PyTorch reimplementation of KataGo, resizing the inputs and outputs for my data. I really needed this to work, since I had no other ideas. It did.

Compared to the Keras net, this net resulted in a mean score of approx. 50. And finally could I complete the cycle by feeding the new game records back to the neural net training, now that I'd gotten around the DL4J speed issue. Once I had 120,000 of them (a conservatively high guess at how many I needed), I did so, producing a new net that made the engine improve to 56. With 120,000 of these games, I trained a net that improved the engine to 64. (I trained a new net from scratch each time, because I was worried the old data might be too different from the new data for continuing training the old net on the new data to be optimal.)

Here's a sample of the games averaging 56:
![games averaging 56](https://cdn.discordapp.com/attachments/778845712053305375/1018638638944366633/57.png)

For the next iteration, I wanted to try a larger net, so I waited two whole weeks until I had 200,000 new game records, to be extra safe. I trained a b6c96 net and a b10c128 net, and to my surprise, their strength was not significantly different! (The mean score using the 10-block net was 0.1 higher, which was not statistically significant for my sample.) So tried smaller nets instead, and I found that I could go down as far as b2c64 without significant decrease in strength. The mean score using all of these nets was approx. 67.3.

With the smaller net, rollouts were faster, so I could increase the rollouts per game, though I did so gradually, enjoying at first the great speedup in generating games. Also, a result of the GPU speedup was that in the last part of the rollouts after the last random draw from the deck, which I call the endgame, where I had been using CPU only, not consulting the net, this CPU usage became a bottleneck until I wrote code to solve certain trivial endgames in which a one-cell shape is easily proved optimal. (In 1/100 games, the bot would draw a shoreline that gave +1 sand point and -2 boat points, presumably having a blind spot for 0-point shorelines in such positions, which the trivial endgame solver would find. Thus there is a 0.01 mean score improvement from the trivial endgame solver, in addition to the speedup.)

After 67.3, the mean progressed to 68.6, 69.6, 70.1, 70.5, as I generated more game records, trained new nets, and also gradually increased the rollouts per game. At some point in that progression, I also began to feel that it was safe to train existing nets on new data, which I did, though the 70.5 is the result of a new net from scratch. Also, I think all or most of the improvement from 70.1 is from increasing the rollouts (I'm not sure because I haven't tested the previous rollout setting), which would mean that the net stopped improving, which would be the sign that I'm done. However, there is a complication.

**TinyBot's policy blind spot: early lakes**

Here's a sample of the games averaging 70.5:
![games averaging 70.5](https://cdn.discordapp.com/attachments/778845712053305375/1030891872366903418/70.png)

The bot always draws two major shorelines early, then a simple one at the end. Sometimes, when the bot has already placed a water tile on the first island, then the simple shoreline will almost certainly be a lake for that water tile, unless some 1/100000 crazy thing happens elsewhere on the board. In this case, it would be better for the second shoreline to be the lake, leaving the last shoreline undefined until the endgame, a great gain in flexibility.

Currently, the main reason the bot doesn't do it is because no such moves occur in the training data, the previous set of game records. There still remains to be explained the times when, due to noise added to the neural net output to explore novel moves, the move is "suggested" to the bot and the bot must have declined. Also, in the old data, sometimes the second shoreline is simple (though never a lake in the 20-game sample in the previous screenshot) and the last shoreline more complex. So why did the bot stop playing this way?

It's because when the bot is drawing the second shoreline, when it looks ahead into the future using rollouts, it sees an endgame played by pure tree search, not guided by neural net after the first cell is chosen. (It doesn't understand that it will have the guidance when it actually reaches endgame.) It appears to the bot that the shoreline it can think of now is superior to the shoreline drawn by pure search, even though the former is a significant constraint on the last third of the tile placements, and the latter isn't.

I'm now generating a set of game records in which the bot is forced to draw a lake with the second shoreline whenever there are water tiles on the first island. After I next train a new net with this data, I also have a new endgame search improvement to test, which attempts to generate guidance based on the final water-or-land predictions (from what used to be KataGo's ownership head) at the the start of the endgame.

What I don't want to do is to code a good-old-fashioned heuristic-rule-based endgame solver (like the trivial endgame solver, only for all endgames).

(But, dear reader, I wouldn't say no if you wanted to code one for me.)
