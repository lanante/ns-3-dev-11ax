#!/bin/bash
# Generate the plots for Study 1 parametric study.
# For each area of sensitivity analysis, this will combine
# the results from several specific individual simulations into
# one chart.

export LC_NUMERIC="en_US.UTF-8"

set -o errexit

# params that remain constant
d=34.64
r=10
nBss=7

# ECDF of System Throughput as a distribution of each STA
index=0
for n in 5 20 ; do
    for ap in ap1 ; do
        for offeredLoad in 2.0 ; do
            ol1=$(awk "BEGIN {print $offeredLoad*1.0}")
            # uplink is 90% of total offered load
            uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
            # downlink is 10% of total offered load
            downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
            d1=$(awk "BEGIN {print $d*100}")
            ul1=$(awk "BEGIN {print $uplink*100}")
            dl1=$(awk "BEGIN {print $downlink*100}")
            patt=$(printf "%0.f-%02d-%02d-%0.2g-%0.1f-%0.1f\n" ${d1} ${r} ${n} ${ol1} ${ul1} ${dl1})
            echo "pattern=$patt"
            rm -f ./results/plot_tmp.dat
            # Nodes 7-11 are the n=5 STAs for AP1
            for i in $(seq 1 $n); do
                nodeIdx=$(awk "BEGIN {print $i+6}")
                grepPattern="Node $nodeIdx,"
                grep "$grepPattern" "./results/spatial-reuse-SR-stats-study1-$patt.dat" >> ./results/plot_tmp.dat
            done

            rm -f "xxx-$patt-$ap-ecdf.dat"

            # the file used by ecdf2.py needs a header row
            echo "0, 0, 0, 0, 0, 0, 0, 0, 0" >> "xxx-$patt-$ap-ecdf.dat"

            while read p ; do
                IFS=','; arrP=($p); unset IFS;
                F="${arrP[3]}"
                IFS=' '; arrF=($F); unset IFS;
                # prepare the output file for the ecdf2.py script
                echo "0, 0, 0, 0, ${arrF[2]}, ${arrP[0]}, 0, 0, 0" >> "xxx-$patt-$ap-ecdf.dat"
            done <./results/plot_tmp.dat

            # generate ECDF plots using the python script
            echo "plotting"
            python ecdf2.py "xxx-$patt-$ap-ecdf.dat" 4 0 0 0 0 "./results/study1-$patt-$ap-ecdf.png" 0

            index=`expr $index + 1`
        done
    done
done
rm -f xxx-*.dat
