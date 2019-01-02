#!/bin/bash
# plot the performance measures for
# each individual simulation scenario that was run

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

# Throughput - each AP on its own chart

# params that will vary
for d in 20 40 60 80 ; do
    for r in 10 20 30 ; do
        for nBss in 2 ; do
            for n in 5 10 20 ; do
                for ap in ap1 ap2 ; do
                    patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
                    echo "pattern=$patt"
                    grep "$patt" "./results/spatial-reuse-experiments-throughput-$ap.dat" > ./results/plot_tmp.dat

                    rm -f xxx.dat

                    while read p ; do
                        #  echo "$p"
                        IFS=':'; arrP=($p); unset IFS;
                        F="${arrP[0]}"
                        IFS='-'; arrF=($F); unset IFS;
                        echo "${arrF[9]}, ${arrP[2]}"
                        echo "${arrF[9]}, ${arrP[2]}" >> xxx.dat
                    done <./results/plot_tmp.dat

	    	    echo "plotting"

                    index=0
                    FILES[$index]=xxx.dat
                    YCOLS[$index]='($2)'
                    XCOLS[$index]='($1)'
                    LABELS[$index]="$ap"

                    PLOTTYPE="with linespoints"
                    XLABEL="Offered Load [Mbps]"
                    YLABEL="Throughput [Mbps]"
                    RANGE=$THROUGHPUTF_RANGE
                    OPTIONS="$BASE_OPTIONS ; set key bottom right"
                    IMGFILENAME="throughput-$patt-$ap"
                    plot

                    unset FILES
                    unset LABELS
                    unset YCOLS
                    unset XCOLS

                done
            done
        done
    done
done

# Throughput - both APs on one chart

# params that will vary
for d in 20 40 60 80 ; do
    for r in 10 20 30 ; do
        for nBss in 2 ; do
            for n in 5 10 20 ; do
                index=0
                for ap in ap1 ap2 ; do
                    patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
                    echo "pattern=$patt"
                    grep "$patt" "./results/spatial-reuse-experiments-throughput-$ap.dat" > ./results/plot_tmp.dat

                    rm -f "xxx-$ap.dat"

                    while read p ; do
                        #  echo "$p"
                        IFS=':'; arrP=($p); unset IFS;
                        F="${arrP[0]}"
                        IFS='-'; arrF=($F); unset IFS;
                        echo "${arrF[9]}, ${arrP[2]}"
                        echo "${arrF[9]}, ${arrP[2]}" >> "xxx-$ap.dat"
                    done <./results/plot_tmp.dat

	    	    echo "plotting"

                    FILES[$index]="xxx-$ap.dat"
                    YCOLS[$index]='($2)'
                    XCOLS[$index]='($1)'
                    LABELS[$index]="$ap"

                    index=`expr $index + 1`

                done

                PLOTTYPE="with linespoints"
                XLABEL="Offered Load [Mbps]"
                YLABEL="Throughput [Mbps]"
                RANGE=$THROUGHPUTF_RANGE
                OPTIONS="$BASE_OPTIONS ; set key bottom right"
                IMGFILENAME="throughput-$patt-both"
                plot

                unset FILES
                unset LABELS
                unset YCOLS
                unset XCOLS

            done
        done
    done
done

# Area Capacity - each AP on its own chart

# params that will vary
for d in 20 40 60 80 ; do
    for r in 10 20 30 ; do
        for nBss in 2 ; do
            for n in 5 10 20 ; do
                for ap in ap1 ap2 ; do
                    patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
                    echo "pattern=$patt"
                    grep "$patt" "./results/spatial-reuse-experiments-area-capacity-$ap.dat" > ./results/plot_tmp.dat

                    rm -f xxx.dat

                    while read p ; do
                        #  echo "$p"
                        IFS=':'; arrP=($p); unset IFS;
                        F="${arrP[0]}"
                        IFS='-'; arrF=($F); unset IFS;
                        echo "${arrF[9]}, ${arrP[2]}"
                        echo "${arrF[9]}, ${arrP[2]}" >> xxx.dat
                    done <./results/plot_tmp.dat

	    	    echo "plotting"

                    index=0
                    FILES[$index]=xxx.dat
                    YCOLS[$index]='($2)'
                    XCOLS[$index]='($1)'
                    LABELS[$index]="$ap"

                    PLOTTYPE="with linespoints"
                    XLABEL="Offered Load [Mbps]"
                    YLABEL="Area Capacity [Mbps/m^2]"
                    RANGE=$AREA_CAPACITY_RANGE
                    OPTIONS="$BASE_OPTIONS ; set key bottom right"
                    IMGFILENAME="area-capacity-$patt-$ap"
                    plot

                    unset FILES
                    unset LABELS
                    unset YCOLS
                    unset XCOLS

                done
            done
        done
    done
done

# Area Capacity - both APs on one chart

# params that will vary
for d in 20 40 60 80 ; do
    for r in 10 20 30 ; do
        for nBss in 2 ; do
            for n in 5 10 20 ; do
                index=0
                for ap in ap1 ap2 ; do
                    patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
                    echo "pattern=$patt"
                    grep "$patt" "./results/spatial-reuse-experiments-area-capacity-$ap.dat" > ./results/plot_tmp.dat

                    rm -f "xxx-$ap.dat"

                    while read p ; do
                        #  echo "$p"
                        IFS=':'; arrP=($p); unset IFS;
                        F="${arrP[0]}"
                        IFS='-'; arrF=($F); unset IFS;
                        echo "${arrF[9]}, ${arrP[2]}"
                        echo "${arrF[9]}, ${arrP[2]}" >> "xxx-$ap.dat"
                    done <./results/plot_tmp.dat

	    	    echo "plotting"

                    FILES[$index]="xxx-$ap.dat"
                    YCOLS[$index]='($2)'
                    XCOLS[$index]='($1)'
                    LABELS[$index]="$ap"

                    index=`expr $index + 1`

                done

                PLOTTYPE="with linespoints"
                XLABEL="Offered Load [Mbps]"
                YLABEL="Area Capacity [Mbps/m^2]"
                RANGE=$AREA_CAPACITY_RANGE
                OPTIONS="$BASE_OPTIONS ; set key bottom right"
                IMGFILENAME="area-capacity-$patt-both"
                plot

                unset FILES
                unset LABELS
                unset YCOLS
                unset XCOLS

            done
        done
    done
done


# Spectrum Efficiency - each AP on its own chart

# params that will vary
for d in 20 40 60 80 ; do
    for r in 10 20 30 ; do
        for nBss in 2 ; do
            for n in 5 10 20 ; do
                for ap in ap1 ap2 ; do
                    patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
                    echo "pattern=$patt"
                    grep "$patt" "./results/spatial-reuse-experiments-spectrum-efficiency-$ap.dat" > ./results/plot_tmp.dat

                    rm -f xxx.dat

                    while read p ; do
                        #  echo "$p"
                        IFS=':'; arrP=($p); unset IFS;
                        F="${arrP[0]}"
                        IFS='-'; arrF=($F); unset IFS;
                        echo "${arrF[9]}, ${arrP[2]}"
                        echo "${arrF[9]}, ${arrP[2]}" >> xxx.dat
                    done <./results/plot_tmp.dat

	    	    echo "plotting"

                    index=0
                    FILES[$index]=xxx.dat
                    YCOLS[$index]='($2)'
                    XCOLS[$index]='($1)'
                    LABELS[$index]="$ap"

                    PLOTTYPE="with linespoints"
                    XLABEL="Offered Load [Mbps]"
                    YLABEL="Spectrum Efficiency [Mbps//Hz/m^2]"
                    RANGE=$SPECTRUM_EFFICIENCY_RANGE
                    OPTIONS="$BASE_OPTIONS ; set key bottom right"
                    IMGFILENAME="spectrum-efficiency-$patt-$ap"
                    plot

                    unset FILES
                    unset LABELS
                    unset YCOLS
                    unset XCOLS

                done
            done
        done
    done
done


# Spectrum Efficiency - both APs on one chart

# params that will vary
for d in 20 40 60 80 ; do
    for r in 10 20 30 ; do
        for nBss in 2 ; do
            for n in 5 10 20 ; do
                index=0
                for ap in ap1 ap2 ; do
                    patt=$(printf "%02d-%02d-%02d-%02d" ${d} ${r} ${nBss} ${n})
                    echo "pattern=$patt"
                    grep "$patt" "./results/spatial-reuse-experiments-spectrum-efficiency-$ap.dat" > ./results/plot_tmp.dat

                    rm -f "xxx-$ap.dat"

                    while read p ; do
                        #  echo "$p"
                        IFS=':'; arrP=($p); unset IFS;
                        F="${arrP[0]}"
                        IFS='-'; arrF=($F); unset IFS;
                        echo "${arrF[9]}, ${arrP[2]}"
                        echo "${arrF[9]}, ${arrP[2]}" >> "xxx-$ap.dat"
                    done <./results/plot_tmp.dat

	    	    echo "plotting"

                    FILES[$index]="xxx-$ap.dat"
                    YCOLS[$index]='($2)'
                    XCOLS[$index]='($1)'
                    LABELS[$index]="$ap"

                    index=`expr $index + 1`

                done

                PLOTTYPE="with linespoints"
                XLABEL="Offered Load [Mbps]"
                YLABEL="Spectrum Efficiency [Mbps//Hz/m^2]"
                RANGE=$SPECTRUM_EFFICIENCY_RANGE
                OPTIONS="$BASE_OPTIONS ; set key bottom right"
                IMGFILENAME="spectrum-efficiency-$patt-both"
                plot

                unset FILES
                unset LABELS
                unset YCOLS
                unset XCOLS

            done
        done
    done
done
