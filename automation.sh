#!/bin/bash

CITY=$1
if [ -z "$CITY" ]; then
    echo "Usage: $0 <City_Name>"
    exit 1
fi

for i in {1..5}
do
make clean
make
sleep 3
./weather_app "$CITY"
done
make clean
