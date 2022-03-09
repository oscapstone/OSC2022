#!/bin/bash

CONTAINER=$(sudo docker ps -a -f name=OSDI -q)
RUNNING=$(sudo docker ps -f name=OSDI -q)

if [[ -z "$CONTAINER" ]]
then
    sudo docker run -d --name=OSDI --privileged -v $PWD/osdi2022:/home/osdi osdi2022:latest /bin/bash
else
    if [[ -z "$RUNNING" ]]
    then
        sudo docker start $CONTAINER
    fi
    sudo docker exec -it $CONTAINER /bin/bash
fi