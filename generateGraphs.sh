

cd results || exit

mkdir graphs &> /dev/null
for i in * ; do
  if [ $(basename "$i") == "graphs" ] ; then
    continue;
  fi
  ( ./../makeAGraph.sh $(pwd)'/'"$i" ) &
done
