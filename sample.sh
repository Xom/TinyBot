while true
do
  rg $1 test.txt | shuf -n 1 | ./cmake-build-debug/tinybot
  read -d ' ' key
done
