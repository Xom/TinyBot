TinyBot game records

Explanation of filenames

`tb` stands for TinyBot. There are three files named like `${mean}_${n}.txt` containing miscellaneous game records, mostly from the 2020 Java version of TinyBot. `tb00.txt` is the output of a MCTS-only version of TinyBot, with no neural net. All subsequent record files, named like `tb${i}.txt`, are from incorporating a neural net trained on the `tb${i-1}.txt` (and in some cases also `tb${i-2}.txt`). Files named like `tb${i}_32.txt` are from running with reduced strength (i.e. max 32 playouts per candidate). `tb11t.txt` (the main result file) and other filenames with a `t` contain transcribed shuffles instead of random naive shuffles (see toplevel READMEs). `tb11_0` refers to the search-less pure-policy run with the final net. (The final net's number is 10 because it was trained on `tb10.txt`.) `tb11i.txt` contain statistically invalid results; that is, it contains completed games from batches interrupted by crashes, which are statistically invalid because they include shorter games and exclude longer games. `tb11u.txt` contains statistically invalid results on transcribed shuffles.

Older records may not be compatible with the HTML-based viewer.
