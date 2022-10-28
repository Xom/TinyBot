#!/usr/bin/env bash

for i in {0..92..4}
do
    let "end = $i + 4"
    python shuffle_0.py 300 $i $end
    echo $i $end
done
