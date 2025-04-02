#!/bin/bash

service_time=5
batches="2 4 8 16 32"
nrings="2 4 8 16 18"

mkdir -p results

# Bench default SPMC
./bin/default_program -b 1 -p 1 -s $service_time -c 17 > results/ ssrb.csv

# Bench batched SPMC
for batch in $batches; do
    ./bin/batched_program -l $batch -p 1 -s $service_time -c 17 > results/ ssrb-bc-$batch.csv
done

# Bench SPMC with nrings
for nr in $nrings; do
    ./bin/multi_program -b 1 -p 1 -s $service_time -n $nr -c 17 > results/ mrb-$nr.csv
done

# Bench noqueue 
./bin/noqueue_program -b 1 -p 17 -s $service_time -c 17 > results/ noqueue.csv
