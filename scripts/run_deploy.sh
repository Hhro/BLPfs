#!/bin/bash
# bash generate random 32 character alphanumeric string (lowercase only)
RAND=$(cat /dev/urandom | tr -dc 'a-z0-9' | fold -w 16 | head -n 1)

fname="/tmp/blpfs_"$RAND

cd /home/cykor
./backend/fs_gen $fname $1
./backend/blpfsm $fname 
