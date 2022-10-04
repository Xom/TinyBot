#!/usr/bin/env bash

for i in {0..56..4}
do
    let "end = $i + 4"
    python shuffle_0.py 160 $i $end
    echo $i $end
done
