#!/usr/bin/env bash

headers=$(find ../../simulation/include | grep .hpp)

echo "headers $headers"

for header in $headers; do
	rootname=$(echo "$header" | sed 's/^.*engine\///g' | sed 's/\.hpp/.puml/g')
	echo "header $header"
	echo "rootname $rootname"
	hpp2plantuml -i "$header" -o "$rootname"
	plantuml "$rootname"
	rm $rootname
done
