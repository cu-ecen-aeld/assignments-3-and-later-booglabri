#!/usr/bin/env bash

if [ $# -ne 2 ]
then
    echo "ERROR: Invalid Number of Arguments."
    echo "Total number arguments should be 2."
    echo "The order of the arguments should be:"
    echo "   1) File and path to write to"
    echo "   2) String to be written to file" 
    exit 1
fi

writefile=$1
writestr=$2
writedir=`dirname "${writefile}"`

if [ ! -d ${writedir} ]
then
    mkdir -p ${writedir}
fi

echo ${writestr} > ${writefile}

if [ $? -ne 0 ]
then
    echo "ERROR: File could not be created."
    exit 1
fi

exit 0
