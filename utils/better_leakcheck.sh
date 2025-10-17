#!/bin/bash

all_passed=true

for f in ./*; do
    extension="${f##*.}"
    [ "$extension" != "sh" ] || continue
    [ "$extension" != "sh~" ] || continue
    [ -f "$f" ] || continue
    [ -x "$f" ] || continue

    output=$(valgrind --leak-check=full "$f" 2>&1)
    if echo "$output" | grep -q "All heap blocks were freed -- no leaks are possible"; then
        echo "$f: passed"
    else
        echo "$f: failed"
        all_passed=false
    fi
done

if [ "$all_passed" = true ]; then
    echo "Memory leak check passed"
else
    echo "Memory leak check failed"
fi

