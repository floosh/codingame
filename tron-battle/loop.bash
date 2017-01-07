#!/bin/bash

if [ $# -eq 0 ]
then
    echo "Usage: loop.bash NUMBER_OF_GAMES"
    exit 1
fi

echo 'gonna do $1 iterations' 
win=0

for ((n=0;n<$1;n++));
do
  result=`node arena.js`
  result="${result:0:1}"
  if [[ "$result" -eq "1" ]]; then
    ((win++))
  fi
done

echo $win
