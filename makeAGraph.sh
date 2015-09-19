##echo $(pwd)
cd "$1"
echo $(pwd)
FILE_NAME=$(basename "$1")
gnuplot -e "set title \"$FILE_NAME\" ; set output \"../graphs/$FILE_NAME.svg\"" ../../makeAGraph.gp
