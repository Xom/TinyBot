#!/usr/bin/env bash

for i in {0..159}
do
    python shuffle_1.py $i {0..52..4}
    echo $i
    df -h
done
