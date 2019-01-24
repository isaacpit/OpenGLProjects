#!/bin/bash

# tool to help with compiling CSCE 441 Assignment 1
# usage: 
#   sh test.sh [<width> <height> [ <file> ]]
m=0
if [ $# -eq 0 ]; then 
  printf "no width / height supplied"
  w=100
  h=100
  f="tri"
elif [ $# -eq 2 ]; then
  w=$1
  h=$2
  f="tri"
elif [ $# -eq 3 ]; then
  w=$1
  h=$2
  f=$3 
elif [ $# -eq 4 ]; then
  w=$1
  h=$2
  f=$3
  m=$4
fi

printf "working with file $f"
make -j4 
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
echo "cat ../resources/$f.obj"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
cat ../resources/$f.obj
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
printf "./A1 ../resources/$f.obj $f.png $w $h $m\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
./A1 ../resources/$f.obj $f.png $w $h $m 
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
open $f.png
