#!/bin/bash
# Generate the plots for sensitivity studies.
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

# Area Capacity - two scenarios, all (4) APs on one chart

# params that will vary
index=0
for patt in 80-30-02-20 20-10-02-05 ; do
                for ap in ap1 ap2 ; do
                    # patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
                    echo "pattern=$patt"
                    grep "$patt" "./results/spatial-reuse-experiments-area-capacity-$ap.dat" > ./results/plot_tmp.dat

                    rm -f "xxx-$patt-$ap.dat"

                    while read p ; do
                        #  echo "$p"
                        IFS=':'; arrP=($p); unset IFS;
                        F="${arrP[0]}"
                        IFS='-'; arrF=($F); unset IFS;
                        echo "${arrF[9]}, ${arrP[2]}"
                        echo "${arrF[9]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
                    done <./results/plot_tmp.dat

	    	    echo "plotting"

                    FILES[$index]="xxx-$patt-$ap.dat"
                    YCOLS[$index]='($2)'
                    XCOLS[$index]='($1)'
                    LABELS[$index]="$patt-$ap"

                    index=`expr $index + 1`

                done

done

PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Area Capacity [Mbps/m^2]"
RANGE=$AREA_CAPACITY_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="area-capacity-combined1"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS

# Area Capacity - varying d.

# params that will vary
index=0
for patt in 80-20-02-10 60-20-02-10 40-20-02-10 20-20-02-10 ; do
    for ap in ap1 ap2 ; do
        # patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
        echo "pattern=$patt"
        grep "$patt" "./results/spatial-reuse-experiments-area-capacity-$ap.dat" > ./results/plot_tmp.dat

        rm -f "xxx-$patt-$ap.dat"

        while read p ; do
            #  echo "$p"
            IFS=':'; arrP=($p); unset IFS;
            F="${arrP[0]}"
            IFS='-'; arrF=($F); unset IFS;
            echo "${arrF[9]}, ${arrP[2]}"
            echo "${arrF[9]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
        done <./results/plot_tmp.dat

        echo "plotting"

        FILES[$index]="xxx-$patt-$ap.dat"
        YCOLS[$index]='($2)'
        XCOLS[$index]='($1)'
        LABELS[$index]="$patt-$ap"

        index=`expr $index + 1`

    done

done

PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Area Capacity [Mbps/m^2]"
RANGE=$AREA_CAPACITY_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="area-capacity-distance"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS

# Area Capacity - varying r

# params that will vary
index=0
for patt in 80-30-02-10 80-20-02-10 80-10-02-10 ; do
    for ap in ap1 ap2 ; do
        # patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
        echo "pattern=$patt"
        grep "$patt" "./results/spatial-reuse-experiments-area-capacity-$ap.dat" > ./results/plot_tmp.dat

        rm -f "xxx-$patt-$ap.dat"

        while read p ; do
            #  echo "$p"
            IFS=':'; arrP=($p); unset IFS;
            F="${arrP[0]}"
            IFS='-'; arrF=($F); unset IFS;
            echo "${arrF[9]}, ${arrP[2]}"
            echo "${arrF[9]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
        done <./results/plot_tmp.dat

        echo "plotting"

        FILES[$index]="xxx-$patt-$ap.dat"
        YCOLS[$index]='($2)'
        XCOLS[$index]='($1)'
        LABELS[$index]="$patt-$ap"

        index=`expr $index + 1`

    done

done


PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Area Capacity [Mbps/m^2]"
RANGE=$AREA_CAPACITY_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="area-capacity-radius"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS

# Area Capacity - varying n

# params that will vary
index=0
for patt in 80-20-02-20 80-20-02-10 80-20-02-05 ; do
    for ap in ap1 ap2 ; do
        # patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
        echo "pattern=$patt"
        grep "$patt" "./results/spatial-reuse-experiments-area-capacity-$ap.dat" > ./results/plot_tmp.dat

        rm -f "xxx-$patt-$ap.dat"

        while read p ; do
            #  echo "$p"
            IFS=':'; arrP=($p); unset IFS;
            F="${arrP[0]}"
            IFS='-'; arrF=($F); unset IFS;
            echo "${arrF[9]}, ${arrP[2]}"
            echo "${arrF[9]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
        done <./results/plot_tmp.dat

        echo "plotting"

        FILES[$index]="xxx-$patt-$ap.dat"
        YCOLS[$index]='($2)'
        XCOLS[$index]='($1)'
        LABELS[$index]="$patt-$ap"

        index=`expr $index + 1`

    done

done


PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Area Capacity [Mbps/m^2]"
RANGE=$AREA_CAPACITY_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="area-capacity-nSTAs"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS

# Area Capacity - varying r, v2 for the Introduction section

# params that will vary
index=0
for patt in 80-30-02-10 80-10-02-10 ; do
    for ap in ap1 ap2 ; do
        # patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
        echo "pattern=$patt"
        grep "$patt" "./results/spatial-reuse-experiments-area-capacity-$ap.dat" > ./results/plot_tmp.dat

        rm -f "xxx-$patt-$ap.dat"

        while read p ; do
            #  echo "$p"
            IFS=':'; arrP=($p); unset IFS;
            F="${arrP[0]}"
            IFS='-'; arrF=($F); unset IFS;
            echo "${arrF[9]}, ${arrP[2]}"
            echo "${arrF[9]}, ${arrP[2]}" >> "xxx-$patt-$ap.dat"
        done <./results/plot_tmp.dat

        echo "plotting"

        FILES[$index]="xxx-$patt-$ap.dat"
        YCOLS[$index]='($2)'
        XCOLS[$index]='($1)'
        LABELS[$index]="$patt-$ap"

        index=`expr $index + 1`

    done

done


PLOTTYPE="with linespoints"
XLABEL="Offered Load [Mbps]"
YLABEL="Area Capacity [Mbps/m^2]"
RANGE=$AREA_CAPACITY_RANGE
OPTIONS="$BASE_OPTIONS ; set key top left"
IMGFILENAME="area-capacity-radius-v2"
plot

unset FILES
unset LABELS
unset YCOLS
unset XCOLS
