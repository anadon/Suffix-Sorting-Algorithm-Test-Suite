

cd results || exit

mkdir graphs &> /dev/null
for i in * ; do
  ./../makeAGraph.sh $i &
done
