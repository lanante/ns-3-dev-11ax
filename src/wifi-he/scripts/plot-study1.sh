#!/bin/bash
# Generate the plots for Study 1 parametric study.
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
THROUGHPUT_RANGE="[1:12][1:10]"
AREA_CAPACITY_RANGE="[1:12][0:0.05]"
SPECTRUM_EFFICIENCY_RANGE="[1.0:12.0][0:0.002]"
AIRTIME_UTILIZATION_RANGE="[1.0:12.0][0:100]"

# params that remain constant
d=34.64
r=10
nBss=7

# System Throughput, as n varies, all on one chart

# params that will vary
index=0
for n in 5 10 15 20 25 30 35 40 ; do
    d1=$(awk "BEGIN {print $d*100}")
    patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
    echo "pattern=$patt"
    for ap in ap1 ap2 ap2 ap3 ap4 ap5 ap6 ap7; do
        grep "$patt" "./results/spatial-reuse-study1-throughput-$ap.dat" > ./results/plot_tmp.dat

        rm -f "xxx-$patt-$ap.dat"
        while read p ; do
            IFS=':'; arrP=($p); unset IFS;
            F="${arrP[0]}"
            IFS='-'; arrF=($F); unset IFS;
            echo "${arrF[8]} ${arrP[2]}" >> "xxx-$patt-$ap.dat"
        done <./results/plot_tmp.dat

        # sort the data points before plotting
        sort -n -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
    done

    rm -f xxx-$patt.dat
    declare -a sum
    for ap in ap1 ap2 ap3 ap4 ap5 ap6 ap7; do
        while read a b; do
            if [ -z ${sum[a]} ]; then
                sum[a]=$b
            else
                sum[a]="$( bc <<<"${sum[a]} + $b" )"
            fi
        done <xxx-$patt-$ap.dat
    done

    for idx in ${!sum[*]}; do
        sum[$idx]="$( bc <<<"scale=6; ${sum[$idx]} / 7.0" )"
        echo "$idx, ${sum[$idx]}"
        echo  "$idx, ${sum[$idx]}" >> "xxx-$patt.dat"
    done
    unset sum

    echo "plotting"

    FILES[$index]="xxx-$patt.dat"
    YCOLS[$index]='($2)'
    XCOLS[$index]='($1)'
    LABELS[$index]="n=$n"

    index=`expr $index + 1`
done

PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Throughput [Mbps]"
RANGE=$THROUGHPUT_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="throughput-study1"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS

# Area Capacity, as n varies, all on one chart

# params that will vary
index=0
for n in 5 10 15 20 25 30 35 40 ; do
    d1=$(awk "BEGIN {print $d*100}")
    patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
    echo "pattern=$patt"
    for ap in ap1 ap2 ap2 ap3 ap4 ap5 ap6 ap7; do
        grep "$patt" "./results/spatial-reuse-study1-area-capacity-$ap.dat" > ./results/plot_tmp.dat

        rm -f "xxx-$patt-$ap.dat"
        while read p ; do
            IFS=':'; arrP=($p); unset IFS;
            F="${arrP[0]}"
            IFS='-'; arrF=($F); unset IFS;
            echo "${arrF[8]} ${arrP[2]}" >> "xxx-$patt-$ap.dat"
        done <./results/plot_tmp.dat

        # sort the data points before plotting
        sort -n -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
    done

    rm -f xxx-$patt.dat
    declare -a sum
    for ap in ap1 ap2 ap3 ap4 ap5 ap6 ap7; do
        while read a b; do
            if [ -z ${sum[a]} ]; then
                sum[a]=$b
            else
                sum[a]="$( bc <<<"${sum[a]} + $b" )"
            fi
        done <xxx-$patt-$ap.dat
    done

    for idx in ${!sum[*]}; do
        sum[$idx]="$( bc <<<"scale=6; ${sum[$idx]} / 7.0" )"
        echo "$idx, ${sum[$idx]}"
        echo  "$idx, ${sum[$idx]}" >> "xxx-$patt.dat"
    done
    unset sum

    echo "plotting"

    FILES[$index]="xxx-$patt.dat"
    YCOLS[$index]='($2)'
    XCOLS[$index]='($1)'
    LABELS[$index]="n=$n"

    index=`expr $index + 1`
done

PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Area Capacity [Mbps/m^2]"
RANGE=$AREA_CAPACITY_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="area-capacity-study1"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS

# Spectrum Efficiency, as n varies, all on one chart

# params that will vary
index=0
for n in 5 10 15 20 25 30 35 40 ; do
    d1=$(awk "BEGIN {print $d*100}")
    patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
    echo "pattern=$patt"
    for ap in ap1 ap2 ap2 ap3 ap4 ap5 ap6 ap7; do
        grep "$patt" "./results/spatial-reuse-study1-spectrum-efficiency-$ap.dat" > ./results/plot_tmp.dat

        rm -f "xxx-$patt-$ap.dat"
        while read p ; do
            IFS=':'; arrP=($p); unset IFS;
            F="${arrP[0]}"
            IFS='-'; arrF=($F); unset IFS;
            echo "${arrF[8]} ${arrP[2]}" >> "xxx-$patt-$ap.dat"
        done <./results/plot_tmp.dat

        # sort the data points before plotting
        sort -n -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
    done

    rm -f xxx-$patt.dat
    declare -a sum
    for ap in ap1 ap2 ap3 ap4 ap5 ap6 ap7; do
        while read a b; do
            if [ -z ${sum[a]} ]; then
                sum[a]=$b
            else
                sum[a]="$( bc <<<"${sum[a]} + $b" )"
            fi
        done <xxx-$patt-$ap.dat
    done

    for idx in ${!sum[*]}; do
        sum[$idx]="$( bc <<<"scale=6; ${sum[$idx]} / 7.0" )"
        echo "$idx, ${sum[$idx]}"
        echo  "$idx, ${sum[$idx]}" >> "xxx-$patt.dat"
    done
    unset sum

    echo "plotting"

    FILES[$index]="xxx-$patt.dat"
    YCOLS[$index]='($2)'
    XCOLS[$index]='($1)'
    LABELS[$index]="n=$n"

    index=`expr $index + 1`
done

PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Spectrum Efficiency [Mbps/Hz/m^2]"
RANGE=$SPECTRUM_EFFICIENCY_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="spectrum-efficiency-study1"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS

# Air-time Utilization, as n varies, all on one chart

# params that will vary
index=0
for n in 5 10 15 20 25 30 35 40 ; do
    d1=$(awk "BEGIN {print $d*100}")
    patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
    echo "pattern=$patt"
    for ap in ap1 ap2 ap2 ap3 ap4 ap5 ap6 ap7; do
        grep "$patt" "./results/spatial-reuse-study1-airtime-utilization-$ap.dat" > ./results/plot_tmp.dat

        rm -f "xxx-$patt-$ap.dat"
        while read p ; do
            IFS=':'; arrP=($p); unset IFS;
            F="${arrP[0]}"
            IFS='-'; arrF=($F); unset IFS;
            echo "${arrF[8]} ${arrP[2]}" >> "xxx-$patt-$ap.dat"
        done <./results/plot_tmp.dat

        # sort the data points before plotting
        sort -n -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
    done

    rm -f xxx-$patt.dat
    declare -a sum
    for ap in ap1 ap2 ap3 ap4 ap5 ap6 ap7; do
        while read a b; do
            if [ -z ${sum[a]} ]; then
                sum[a]=$b
            else
                sum[a]="$( bc <<<"${sum[a]} + $b" )"
            fi
        done <xxx-$patt-$ap.dat
    done

    for idx in ${!sum[*]}; do
        sum[$idx]="$( bc <<<"scale=6; ${sum[$idx]} / 7.0" )"
        echo "$idx, ${sum[$idx]}"
        echo  "$idx, ${sum[$idx]}" >> "xxx-$patt.dat"
    done
    unset sum

    echo "plotting"

    FILES[$index]="xxx-$patt.dat"
    YCOLS[$index]='($2)'
    XCOLS[$index]='($1)'
    LABELS[$index]="n=$n"

    index=`expr $index + 1`
done


PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Airtime Utilization [%]"
RANGE=$AIRTIME_UTILIZATION_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="airtime-utilization-study1"
plot

wait
rm xxx-3464-10*.dat

unset FILES
unset LABELS
unset YCOLS
unset XCOLS
