#!/usr/bin/env bash

for i in {0..76..4}
do
    let "end = $i + 4"
    python shuffle_0.py 240 $i $end
    echo $i $end
done
