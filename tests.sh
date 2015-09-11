#!/bin/bash
##

for i in bin/* ; do
  [[ -e $i ]] || break
  echo "$i"
  for j in input-files/* ; do
    [[ -e $j ]] || break
    echo "$j"
    for h in {1..100} ; do
      /usr/bin/time -f %M perf stat -e instructions "$i" "$j" |& sed ':a;N;$!ba;s/\n/ /g' | sed -e 's/\s/ /g' | xargs | awk '{ print $7 " " $13 }'
    done
    echo ""
  done
done 
rm perf.data
