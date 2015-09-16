

cd results || exit

mkdir graphs &> /dev/null
for i in * ; do
  cd "$i" || exit
  gnuplot -e "set title '"$i"' ; set term svg ; set output '../graphs/"$i".svg'" -c ../../generateGraphs.gp
  cd ..
done
