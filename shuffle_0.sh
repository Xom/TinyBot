#!/usr/bin/env bash

for i in {40..88..4}
do
    let "end = $i + 4"
    python shuffle_0.py 224 $i $end
    echo $i $end
done
