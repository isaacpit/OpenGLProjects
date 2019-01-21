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
fi


make -j4 
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
echo "cat ../resources/$f.obj"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
cat ../resources/$f.obj
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
printf "./A1 ../resources/$f.obj $f.png $w $h 0\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
./A1 ../resources/$f.obj $f.png $w $h 0 
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' '*' 
open $f.png
