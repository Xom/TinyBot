#!/usr/bin/env bash

for i in {0..24..2}
do
    let "end = $i + 2"
    python shuffle_0.py 100 $i $end
    echo $i $end
done
