while true
do
  cat $1 | shuf -n 1 | ./cmake-build-debug/tinybot
  read -d ' ' key
done
