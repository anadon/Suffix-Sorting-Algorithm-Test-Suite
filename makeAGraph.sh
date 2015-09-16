cd "$1" || exit
gnuplot -e "set title '"$1"' ; set term svg ; set output '../graphs/"$1".svg'" -c ../../generateGraphs.gp

