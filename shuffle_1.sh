#!/usr/bin/env bash

for i in {0..399}
do
    python shuffle_1.py $i {0..96..4}
    echo $i
    df -h
done
