#!/bin/bash
##

mkdir results

for i in bin/* ; do
  [[ -e $i ]] || break
  echo "$i"
  for j in input-files/* ; do
    [[ -e $j ]] || break
    echo "$j"
    if [ ! \( -e "results/$(basename "$i")-$(basename "$j").dat" \) ] ; then
      rm -f "results/$(basename "$i")-$(basename "$j").dat"
      for h in {1..100} ; do
        /usr/bin/time -f %M perf stat -e instructions "$i" "$j"  |& \
        sed ':a;N;$!ba;s/\n/ /g' | sed -e 's/\s/ /g' | sed 's/,//g' | xargs | \
        awk '{ print $7 " " $13 }' &>> "results/$(basename "$i")-$(basename "$j").dat"
      done
    fi
  done
done
