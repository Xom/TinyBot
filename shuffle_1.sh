#!/usr/bin/env bash

for i in {0..99}
do
    python shuffle_1.py $i {0..24..2}
    echo $i
    df -h
done
