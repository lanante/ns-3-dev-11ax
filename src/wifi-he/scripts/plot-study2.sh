#!/bin/bash
# Generate the plots for Study 2 parametric study.
# For each area of sensitivity analysis, this will combine
# the results from several specific individual simulations into
# one chart.

source ../utils/shot
source ../utils/common

IMGDIR=images
THUMBNAILS=true
NOX11=true

BASE_OPTIONS=""
BASE_OPTIONS="${BASE_OPTIONS} ; set style line 1 pt 4 lt 1"
BASE_OPTIONS="${BASE_OPTIONS} ; set style line 2 pt 7 lt 2"
BASE_OPTIONS="${BASE_OPTIONS} ; set style increment ;"

BASE_OPTIONS="${BASE_OPTIONS} ; set pointsize 2 ; set grid;"

PNG_OPTIONS="size 350, 300 font \",9\""

set -o errexit

# below value is used for thumbnail page
lbtTxop=8

# Adjust the scale of the plots here
LATENCY_CDF_RANGE="[0:500][0:1]"
HI_RES_LATENCY_CDF_RANGE="[0:50][0:1]"
THROUGHPUT_CDF_RANGE="[0:150][0:1]"
THROUGHPUT_RANGE="[1:6][1:6]"
AREA_CAPCITY_RANGE="[1:6][0:0.02]"
SPECTRUM_EFFICIENCY_RANGE="[1:6][0:0.002]"
STUDY2_SPECTRUM_EFFICIENCY_RANGE="[1.0:3.0][0:0.00050]"

# params that remain constant
d=17.32
r=10
nBss=7

# System Throughput, as n varies, all on one chart

# params that will vary
index=0
for n in 5 10 15 20 25 30 35 40 ; do
    for ap in ap1 ; do
#        for offeredLoad in 1.0 2.0 3.0 4.0 5.0 6.0 ; do
            # uplink is 90% of total offered load
            # uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
            # downlink is 10% of total offered load
            # downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
            d1=$(awk "BEGIN {print $d*100}")
            # ul1=$(awk "BEGIN {print $uplink*100}")
            # dl1=$(awk "BEGIN {print $downlink*100}")
            # patt=$(printf "%0.f-%02d-%02d-%0.f-%0.f\n" ${d1} ${r} ${n} ${ul1} ${dl1})
            patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
            echo "pattern=$patt"
            grep "$patt" "./results/spatial-reuse-study2-throughput-$ap.dat" > ./results/plot_tmp.dat

            rm -f "xxx-$patt-$ap.dat"

            while read p ; do
                #  echo "$p"
                IFS=':'; arrP=($p); unset IFS;
                F="${arrP[0]}"
                IFS='-'; arrF=($F); unset IFS;
                    echo "${arrF[8]}, ${arrP[2]}"
                    echo "${arrF[8]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
                done <./results/plot_tmp.dat

            # sort the data points before plotting
            sort -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
            echo "plotting"

            FILES[$index]="xxx-$patt-$ap.dat"
            YCOLS[$index]='($2)'
            XCOLS[$index]='($1)'
            LABELS[$index]="n=$n"

            index=`expr $index + 1`
 #       done
    done
done

PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Throughput [Mbps]"
RANGE=$THOUGHPUT_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="throughput-study2-82"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS

# Area Capacity, as n varies, all on one chart

# params that will vary
index=0
for n in 5 10 15 20 25 30 35 40 ; do
    for ap in ap1 ; do
#        for offeredLoad in 1.0 2.0 3.0 4.0 5.0 6.0 ; do
            # uplink is 90% of total offered load
            # uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
            # downlink is 10% of total offered load
            # downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
            d1=$(awk "BEGIN {print $d*100}")
            # ul1=$(awk "BEGIN {print $uplink*100}")
            # dl1=$(awk "BEGIN {print $downlink*100}")
            # patt=$(printf "%0.f-%02d-%02d-%0.f-%0.f\n" ${d1} ${r} ${n} ${ul1} ${dl1})
            patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
            echo "pattern=$patt"
            grep "$patt" "./results/spatial-reuse-study2-area-capacity-$ap.dat" > ./results/plot_tmp.dat

            rm -f "xxx-$patt-$ap.dat"

            while read p ; do
                #  echo "$p"
                IFS=':'; arrP=($p); unset IFS;
                F="${arrP[0]}"
                IFS='-'; arrF=($F); unset IFS;
                    echo "${arrF[8]}, ${arrP[2]}"
                    echo "${arrF[8]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
                done <./results/plot_tmp.dat

            # sort the data points before plotting
            sort -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
            echo "plotting"

            FILES[$index]="xxx-$patt-$ap.dat"
            YCOLS[$index]='($2)'
            XCOLS[$index]='($1)'
            LABELS[$index]="n=$n"

            index=`expr $index + 1`
 #       done
    done
done

PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Area Capacity [Mbps/m^2]"
RANGE=$AREA_CAPACITY_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="area-capacity-study2-82"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS

# Spectrum Efficiency, as n varies, all on one chart

# params that will vary
index=0
for n in 5 10 15 20 25 30 35 40 ; do
    for ap in ap1 ; do
#        for offeredLoad in 1.0 2.0 3.0 4.0 5.0 6.0 ; do
            # uplink is 90% of total offered load
            # uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
            # downlink is 10% of total offered load
            # downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
            d1=$(awk "BEGIN {print $d*100}")
            # ul1=$(awk "BEGIN {print $uplink*100}")
            # dl1=$(awk "BEGIN {print $downlink*100}")
            # patt=$(printf "%0.f-%02d-%02d-%0.f-%0.f\n" ${d1} ${r} ${n} ${ul1} ${dl1})
            patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
            echo "pattern=$patt"
            grep "$patt" "./results/spatial-reuse-study2-spectrum-efficiency-$ap.dat" > ./results/plot_tmp.dat

            rm -f "xxx-$patt-$ap.dat"

            while read p ; do
                #  echo "$p"
                IFS=':'; arrP=($p); unset IFS;
                F="${arrP[0]}"
                IFS='-'; arrF=($F); unset IFS;
                    echo "${arrF[8]}, ${arrP[2]}"
                    echo "${arrF[8]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
                done <./results/plot_tmp.dat

            # sort the data points before plotting
            sort -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
            echo "plotting"

            FILES[$index]="xxx-$patt-$ap.dat"
            YCOLS[$index]='($2)'
            XCOLS[$index]='($1)'
            LABELS[$index]="n=$n"

            index=`expr $index + 1`
 #       done
    done
done

PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Spectrum Efficiency [Mbps/Hz/m^2]"
RANGE=$STUDY2_SPECTRUM_EFFICIENCY_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="spectrum-efficiency-study2-82"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS
