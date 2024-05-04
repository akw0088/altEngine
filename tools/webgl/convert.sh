#!/bin/bash
rm merge.jpg.txt
for file in *.jpg
do
	echo "<!-- $file -->" > "$file".txt
	filename="${file%.*}"
	echo "$filename"
	echo "<img style=\"display: none;\" id=\"$filename\" src=\"data:image/jpeg;base64," >>  "$file".txt
  base64 "$file" >> "$file".txt
  echo "\" />"  >> "$file".txt
  echo "<!-- end $file -->" >> "$file".txt
done
cat *.txt > merge.jpg.txt



