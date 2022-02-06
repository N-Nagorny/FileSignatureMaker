#!/bin/bash

rm -rf splitted/
mkdir splitted/

BLOCK_SIZE=$((1024*1024*$1))
INPUT_PATH=$2
split -b $BLOCK_SIZE "$INPUT_PATH" splitted/

for FILE in splitted/*;
  do md5sum $FILE | awk '{print $1}';
done
