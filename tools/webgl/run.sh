#!bin/bash

for file in *.ibo
do
  ./ibo_to_array "$file"
done
cat *ibo.txt > merge.ibo.txt

