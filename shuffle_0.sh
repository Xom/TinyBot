#!/usr/bin/env bash

for i in {0..96..4}
do
    let "end = $i + 4"
    python shuffle_0.py 400 $i $end
    echo $i $end
done
