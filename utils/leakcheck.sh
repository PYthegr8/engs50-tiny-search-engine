#!/bin/bash


for f in ./*; do
    echo $f
    extension="${f##*.}"
    [ ${extension} != "sh" ] || continue
    [ ${extension} != "sh~" ] || continue
    [ -f "$f" ] || continue
    [ -x "$f" ] || continue

    valgrind --leak-check=full "$f"	
done
