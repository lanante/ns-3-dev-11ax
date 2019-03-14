#!/bin/bash
# Generate the plots for Study 1 parametric study.
# For each area of sensitivity analysis, this will combine
# the results from several specific individual simulations into
# one chart.

export LC_NUMERIC="en_US.UTF-8"

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

# params that remain constant
d=34.64
r=10
nBss=7

# System Throughput, as n varies, all on one chart

for ap in ap1 ; do
    for offeredLoad in 1.0 1.5 2.0 2.5 3.0 3.5 4.0 4.5 5.0 5.5 6.0 ; do
        THROUGHPUT_RANGE="[:][0:5]"
        ol1=$(awk "BEGIN {print $offeredLoad*1.0}")
        uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
        downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
        ul1=$(awk "BEGIN {print $uplink*100}")
        dl1=$(awk "BEGIN {print $downlink*100}")
        patt_offered_load=$(printf "%0.2g-%0.1f-%0.1f" ${ol1} ${ul1} ${dl1})
        if [ -z "$(grep -r $patt_offered_load ./results/spatial-reuse-study2-throughput-$ap.dat)" ]; then
            echo "results for offered load $offeredLoad not available: skip"
            continue
        fi
        echo "Process results for offered load $offeredLoad"

        # params that will vary
        index=0
        for n in 5 10 15 20 25 30 35 40 ; do
            d1=$(awk "BEGIN {print $d*100}")
            patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
            echo "pattern=$patt"
            grep "$patt" "./results/spatial-reuse-study2-throughput-$ap.dat" | grep ${patt_offered_load} > ./results/plot_tmp.dat

            rm -f "xxx-$patt-$ap.dat"

            while read p ; do
                IFS=':'; arrP=($p); unset IFS;
                F="${arrP[0]}"
                IFS='-'; arrF=($F); unset IFS;
                echo "${arrF[12]}, ${arrP[2]}"
                echo "${arrF[12]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
            done <./results/plot_tmp.dat

            # sort the data points before plotting
            sort -n -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
            echo "plotting"

            FILES[$index]="xxx-$patt-$ap.dat"
            YCOLS[$index]='($2)'
            XCOLS[$index]='($1)'
            LABELS[$index]="n=$n"

            index=`expr $index + 1`
        done

        PLOTTYPE="with linespoints"
        XLABEL="OBSS PD Threshold [dBm]"
        YLABEL="Throughput [Mbps]"
        RANGE=$THROUGHPUT_RANGE
        OPTIONS="$BASE_OPTIONS ; set key top left"
        IMGFILENAME="throughput-study2b-${offeredLoad}Mbps"
        plot

        unset FILES
        unset LABELS
        unset YCOLS
        unset XCOLS

    done
done

# Area Capacity, as n varies, all on one chart

for ap in ap1 ; do
    for offeredLoad in 1.0 1.5 2.0 2.5 3.0 3.5 4.0 4.5 5.0 5.5 6.0 ; do
        AREA_CAPACITY_RANGE="[:][0:0.02]"
        ol1=$(awk "BEGIN {print $offeredLoad*1.0}")
        uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
        downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
        ul1=$(awk "BEGIN {print $uplink*100}")
        dl1=$(awk "BEGIN {print $downlink*100}")
        patt_offered_load=$(printf "%0.2g-%0.1f-%0.1f" ${ol1} ${ul1} ${dl1})
        if [ -z "$(grep -r $patt_offered_load ./results/spatial-reuse-study2-area-capacity-$ap.dat)" ]; then
            echo "results for offered load $offeredLoad not available: skip"
            continue
        fi
        echo "Process results for offered load $offeredLoad"

        # params that will vary
        index=0
        for n in 5 10 15 20 25 30 35 40 ; do
            d1=$(awk "BEGIN {print $d*100}")
            patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
            echo "pattern=$patt"
            grep "$patt" "./results/spatial-reuse-study2-area-capacity-$ap.dat" | grep ${patt_offered_load} > ./results/plot_tmp.dat

            rm -f "xxx-$patt-$ap.dat"

            while read p ; do
                IFS=':'; arrP=($p); unset IFS;
                F="${arrP[0]}"
                IFS='-'; arrF=($F); unset IFS;
                echo "${arrF[12]}, ${arrP[2]}"
                echo "${arrF[12]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
            done <./results/plot_tmp.dat

            # sort the data points before plotting
            sort -n -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
            echo "plotting"

            FILES[$index]="xxx-$patt-$ap.dat"
            YCOLS[$index]='($2)'
            XCOLS[$index]='($1)'
            LABELS[$index]="n=$n"

            index=`expr $index + 1`
        done

        PLOTTYPE="with linespoints"
        XLABEL="obssPdThreshold [dBm]"
        YLABEL="Area Capacity [Mbps/m^2]"
        RANGE=$AREA_CAPACITY_RANGE
        OPTIONS="$BASE_OPTIONS ; set key top left"
        IMGFILENAME="area-capacity-study2b-${offeredLoad}Mbps"
        plot

        unset FILES
        unset LABELS
        unset YCOLS
        unset XCOLS

    done
done

# Spectrum Efficiency, as n varies, all on one chart

for ap in ap1 ; do
    for offeredLoad in 1.0 1.5 2.0 2.5 3.0 3.5 4.0 4.5 5.0 5.5 6.0 ; do
        SPECTRUM_EFFICIENCY_RANGE="[:][0:0.002]"
        ol1=$(awk "BEGIN {print $offeredLoad*1.0}")
        uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
        downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
        ul1=$(awk "BEGIN {print $uplink*100}")
        dl1=$(awk "BEGIN {print $downlink*100}")
        patt_offered_load=$(printf "%0.2g-%0.1f-%0.1f" ${ol1} ${ul1} ${dl1})
        if [ -z "$(grep -r $patt_offered_load ./results/spatial-reuse-study2-spectrum-efficiency-$ap.dat)" ]; then
            echo "results for offered load $offeredLoad not available: skip"
            continue
        fi
        echo "Process results for offered load $offeredLoad"

        # params that will vary
        index=0
        for n in 5 10 15 20 25 30 35 40 ; do
            d1=$(awk "BEGIN {print $d*100}")
            patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
            echo "pattern=$patt"
            grep "$patt" "./results/spatial-reuse-study2-spectrum-efficiency-$ap.dat" | grep ${patt_offered_load} > ./results/plot_tmp.dat

            rm -f "xxx-$patt-$ap.dat"

            while read p ; do
                IFS=':'; arrP=($p); unset IFS;
                F="${arrP[0]}"
                IFS='-'; arrF=($F); unset IFS;
                echo "${arrF[12]}, ${arrP[2]}"
                echo "${arrF[12]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
            done <./results/plot_tmp.dat

            # sort the data points before plotting
            sort -n -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
            echo "plotting"

            FILES[$index]="xxx-$patt-$ap.dat"
            YCOLS[$index]='($2)'
            XCOLS[$index]='($1)'
            LABELS[$index]="n=$n"

            index=`expr $index + 1`
        done

        PLOTTYPE="with linespoints"
        XLABEL="obssPdThreshold [dBm]"
        YLABEL="Spectrum Efficiency [Mbps/Hz/m^2]"
        RANGE=$STUDY2_SPECTRUM_EFFICIENCY_RANGE
        OPTIONS="$BASE_OPTIONS ; set key top left"
        IMGFILENAME="spectrum-efficiency-study2b-${offeredLoad}Mbps"
        plot

        unset FILES
        unset LABELS
        unset YCOLS
        unset XCOLS

    done
done

# Air-time Utilization, as n varies, all on one chart

for ap in ap1 ; do
    for offeredLoad in 1.0 1.5 2.0 2.5 3.0 3.5 4.0 4.5 5.0 5.5 6.0 ; do
        AIRTIME_UTILIZATION_RANGE="[:][0:100]"
        ol1=$(awk "BEGIN {print $offeredLoad*1.0}")
        uplink=$(awk "BEGIN {print $offeredLoad*0.9}")
        downlink=$(awk "BEGIN {print $offeredLoad*0.1}")
        ul1=$(awk "BEGIN {print $uplink*100}")
        dl1=$(awk "BEGIN {print $downlink*100}")
        patt_offered_load=$(printf "%0.2g-%0.1f-%0.1f" ${ol1} ${ul1} ${dl1})
        if [ -z "$(grep -r $patt_offered_load ./results/spatial-reuse-study2-airtime-utilization-$ap.dat)" ]; then
            echo "results for offered load $offeredLoad not available: skip"
            continue
        fi
        echo "Process results for offered load $offeredLoad"

        # params that will vary
        index=0
        for n in 5 10 15 20 25 30 35 40 ; do
            d1=$(awk "BEGIN {print $d*100}")
            patt=$(printf "%0.f-%02d-%02d" ${d1} ${r} ${n})
            echo "pattern=$patt"
            grep "$patt" "./results/spatial-reuse-study2-airtime-utilization-$ap.dat" | grep ${patt_offered_load} > ./results/plot_tmp.dat

            rm -f "xxx-$patt-$ap.dat"

            while read p ; do
                IFS=':'; arrP=($p); unset IFS;
                F="${arrP[0]}"
                IFS='-'; arrF=($F); unset IFS;
                echo "${arrF[12]}, ${arrP[2]}"
                echo "${arrF[12]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
            done <./results/plot_tmp.dat

            # sort the data points before plotting
            sort -n -o "xxx-$patt-$ap.dat" "xxx-$patt-$ap.dat"
            echo "plotting"

            FILES[$index]="xxx-$patt-$ap.dat"
            YCOLS[$index]='($2)'
            XCOLS[$index]='($1)'
            LABELS[$index]="n=$n"

            index=`expr $index + 1`
        done

        PLOTTYPE="with linespoints"
        XLABEL="OBSS PD Threshold [dBm]"
        YLABEL="Airtime Utilization [%]"
        RANGE=$AIRTIME_UTILIZATION_RANGE
        OPTIONS="$BASE_OPTIONS ; set key top left"
        IMGFILENAME="airtime-utilization-study2b-${offeredLoad}Mbps"
        plot

        unset FILES
        unset LABELS
        unset YCOLS
        unset XCOLS

    done
done

rm -f xxx-3464-10*.dat
