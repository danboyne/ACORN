#!/bin/sh

for i in *.txt; do
#   sed -i 's/asd/dfg/g' $i
# sed -i 's/^variabilityThreshold = 1000/variabilityThreshold = 5/' $i
# sed -i 's/^maxIterations = 25$/maxIterations = 30/' $i
  sed -i 's/crossingFreeThreshold/violationFreeThreshold/' $i
done
