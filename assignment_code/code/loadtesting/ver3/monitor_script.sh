#!/bin/bash

vmstat 5 >> vmstat_snapshots.txt &

while true; do
    (ps -eLf | grep './server' | head -n 1) >> process_snapshots.txt &
    sleep 5
done
