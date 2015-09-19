echo "$2"
for h in {1..100} ; do
  /usr/bin/time -f %M perf stat -e instructions "$1" "$2"  |& \
  sed ':a;N;$!ba;s/\n/ /g' | sed -e 's/\s/ /g' | sed 's/,//g' | \
  awk '{ print $7 " " $13 }' &>> "results/$(basename "$2")/$(basename "$1").dat"
done
