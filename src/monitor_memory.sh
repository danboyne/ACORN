#!/bin/bash

while true
do
  echo '=================================================='
  date
  echo '  '
  top -b -n 1 -E g -o +RES -u !root | head -n 10
  sleep 120
done
