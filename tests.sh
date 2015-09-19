#!/bin/bash
##

LC_ALL=C

PIDS=""

mkdir results &> /dev/null

for j in input-files/* ; do
  mkdir "results/$(basename "$j")" &> /dev/null
done

for i in bin/* ; do
  echo "$i"
  for j in input-files/* ; do
##    [[ -e "results/$(basename "$j")/$(basename "$i").dat" ]] || continue 
    ./indvTest.sh "$i" "$j" &
     PIDS="$PIDS $!"
  done
done

wait "$PIDS"
echo "Data generation completed."
