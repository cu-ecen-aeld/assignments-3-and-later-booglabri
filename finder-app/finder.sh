#!/usr/bin/env sh

if [ $# -ne 2 ]
then
    echo "ERROR: Invalid Number of Arguments."
    echo "Total number arguments should be 2."
    echo "The order of the arguments should be:"
    echo "   1) File Directory Path."
    echo "   2) String to be searched in specified directory path." 
    exit 1
fi

filesdir=$1
searchstr=$2

if [ ! -d ${filesdir} ]
then
    echo "Error: Directory path ${filesdir} is not a directory."
    exit 1
fi

x=`find ${filesdir} -type f  |wc -l`
y=`find ${filesdir} -type f -exec grep ${searchstr} {} \; |wc -l`

echo "The number of files are ${x} and the number of matching lines are ${y}"
exit 0
