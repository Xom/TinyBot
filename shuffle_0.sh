#!/usr/bin/env bash

for i in {0..51..3}
do
    let "end = $i + 3"
    python shuffle_0.py 200 $i $end
    echo $i $end
done
