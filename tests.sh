#!/bin/bash
##

LC_ALL=C

mkdir results

for j in input-files/* ; do
  mkdir "results/$(basename "$J")" &> /dev/null
done

for i in bin/* ; do
  echo "$i"
  for j in input-files/* ; do
    echo "$j"
    if [ ! \( -e "results/$(basename "$j")/$(basename "$i").dat" \) ] ; then
      for h in {1..100} ; do
        /usr/bin/time -f %M perf stat -e instructions "$i" "$j"  |& \
        sed ':a;N;$!ba;s/\n/ /g' | sed -e 's/\s/ /g' | sed 's/,//g' | \
        awk '{ print $7 " " $13 }' &>> "results/$(basename "$j")/$(basename "$i").dat"
      done
    fi
  done
done
