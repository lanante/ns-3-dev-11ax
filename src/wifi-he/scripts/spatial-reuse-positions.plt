#
set term pngcairo dashed
set output "spatial-reuse-positions.png"
set title "Spatial Reuse Example\nPlot of node positions"
set key outside
set grid
set pointsize
# set size 1, .6 
set size ratio -1.0
#set size ratio 0.5
datafile = 'spatial-reuse-positions.csv'
set parametric
set trange [0:2*pi]
plot 'spatial-reuse-positions.csv' i 0 with points ps 0 pt 13 lc rgb "black" notitle, \
'spatial-reuse-positions.csv' i 1 with points ps 2 pt 9 lc rgb "red" title 'AP A', \
'spatial-reuse-positions.csv' i 2 with points ps 2 pt 11 lc rgb "green" title 'AP B', \
'spatial-reuse-positions.csv' i 3 with points ps 2 pt 12 lc rgb "blue" title 'AP C', \
'spatial-reuse-positions.csv' i 4 with points ps 2 pt 13 lc rgb "black" title 'AP D', \
'spatial-reuse-positions.csv' i 5 with points pt 27 lc rgb "red" title 'STA A', \
'spatial-reuse-positions.csv' i 6 with points pt 28 lc rgb "green" title 'STA B', \
'spatial-reuse-positions.csv' i 7 with points pt 29 lc rgb "blue" title 'STA C', \
'spatial-reuse-positions.csv' i 8 with points pt 30 lc rgb "black" title 'STA D', \
'spatial-reuse-positions.csv' i 1 using 1:2:3 with circles lc rgb "red" notitle, \
'spatial-reuse-positions.csv' i 2 using 1:2:3 with circles lc rgb "green" notitle, \
'spatial-reuse-positions.csv' i 3 using 1:2:3 with circles lc rgb "blue" notitle, \
'spatial-reuse-positions.csv' i 4 using 1:2:3 with circles lc rgb "black" notitle, \
'spatial-reuse-positions.csv' i 1 using 1:4 with points ps 0 pt -1 lc rgb "red" notitle, \
'spatial-reuse-positions.csv' i 1 using 1:(-$4) with points ps 0 pt -1 lc rgb "red" notitle, \
'spatial-reuse-positions.csv' i 1 using 1:2:4 with circles lt 2 dt 2 lc rgb "red" title "A CSR", \
'spatial-reuse-positions.csv' i 2 using 1:4 with points ps 0 pt -1 lc rgb "green" notitle, \
'spatial-reuse-positions.csv' i 2 using 1:(-$4) with points ps 0 pt -1 lc rgb "green" notitle, \
'spatial-reuse-positions.csv' i 2 using 1:2:4 with circles lt 2 dt 2 lc rgb "green" title "B CSR", \
'spatial-reuse-positions.csv' i 3 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
'spatial-reuse-positions.csv' i 3 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
'spatial-reuse-positions.csv' i 3 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "C CSR", \
'spatial-reuse-positions.csv' i 4 using 1:4 with points ps 0 pt -1 lc rgb "black" notitle, \
'spatial-reuse-positions.csv' i 4 using 1:(-$4) with points ps 0 pt -1 lc rgb "black" notitle, \
'spatial-reuse-positions.csv' i 4 using 1:2:4 with circles lt 2 dt 2 lc rgb "black" title "D CSR"
quit
