#!/usr/bin/env bash

for i in {0..99}
do
    if [ $i -eq 0 ]; then
        head -n 2000 $1 | ./cmake-build-debug/tinybot
    else
        let "offset = $i * 2000 + 1"
        tail -n +$offset $1 | head -n 2000 | ./cmake-build-debug/tinybot
    fi
    mkdir shuffle/raw/$i
    mv -t shuffle/raw/$i *.npy
    ls -ahl shuffle/raw/$i
done
