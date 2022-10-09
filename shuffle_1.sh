#!/usr/bin/env bash

for i in {0..239}
do
    python shuffle_1.py $i {0..76..4}
    echo $i
    df -h
done
