#!/usr/bin/env bash

for i in {0..299}
do
    python shuffle_1.py $i {0..92..4}
    echo $i
    df -h
done
