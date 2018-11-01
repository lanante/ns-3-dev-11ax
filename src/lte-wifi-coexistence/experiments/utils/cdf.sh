#!/bin/bash

# From file <filename>, extract a single column of data and generate a
# CDF that is printed to stdout
#
# usage:  cdf.sh <column_number> <filename>

col=$1

awk -v col=$1 '{ print $col }' $2 | sort -g | awk '
{	d[++c] = $0
}
END {	inc = 1 / c
	for(i = 0; i <= c; i++)
		printf("%.4f\t%.4f\n", d[i], i * inc)
}'
