#!/bin/bash

service_time=5
batches="2 4 8 16 32"
nrings="2 4 8 16 18"

mkdir -p results

# Bench default SPMC
echo "Running default SPMC benchmark..."
./bin/default_program -b 1 -p 1 -s $service_time -c 17 > results/ssrb.csv

# Bench batched SPMC
echo "Running batched SPMC benchmark..."
for batch in $batches; do
    echo "Running batch size $batch..."
    ./bin/batched_program -l $batch -p 1 -s $service_time -c 17 > results/ssrb-bc-$batch.csv
done

# Bench SPMC with nrings
echo "Running SPMC with nrings benchmark..."
for nr in $nrings; do
    echo "Running nrings $nr..."
    ./bin/multi_program -b 1 -p 1 -s $service_time -n $nr -c 17 > results/mrb-$nr.csv
done

# Bench noqueue 
echo "Running noqueue benchmark..."
./bin/noqueue_program -b 1 -p 17 -s $service_time -c 17 > results/noqueue.csv
