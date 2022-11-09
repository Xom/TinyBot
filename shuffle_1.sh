#!/usr/bin/env bash

for i in {0..199}
do
    python shuffle_1.py $i {0..50..2}
    echo $i
    df -h
done
