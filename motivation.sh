#!/bin/bash

nconsumers="1 4 8 16 32"
service_times="5 10 15 50 100"

mkdir -p results

echo "Running default SPMC benchmark..."
for consumer in $nconsumers; do
for stime in $service_times; do
    echo "Configuration: stime=$stime consumer=$consumer"
    
    ./bin/default_program -b 1 -p 1 -s $stime -c $consumer -d 5 > results/ssrb-$stime-$consumer.csv
done
done