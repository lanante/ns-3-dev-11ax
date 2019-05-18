#

set term png size 400, 300 font ",8"
set output "spatial-reuse-positions.png"
set title "Spatial Reuse Example\nPlot of node positions"
set key outside font ",8"
set grid
set pointsize
#set size 1, .6 
set size ratio -1.0
#set size 1, 1
#set size ratio 0.5
datafile = 'spatial-reuse-positions.csv'
set parametric
set trange [0:2*pi]

# cannot figure out how to conditionally include certain plots in a multiplot,
# so conditionally generating one plot, based on the value of the variable "nBss"

if (ARG1 == "1") {
  print "Plotting positions for 1 BSS"
  plot \
  'spatial-reuse-positions.csv' i 0 with points ps 0 pt 13 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 with points ps 2 pt 13 lc rgb "red" title 'AP A', \
  'spatial-reuse-positions.csv' i 5 with points pt 28 lc rgb "red" title 'STA A', \
  'spatial-reuse-positions.csv' i 1 using 1:2:3 with circles lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:4 with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:(-$4) with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:2:4 with circles lt 2 dt 2 lc rgb "red" title "A CSR"
}

if (ARG1 == "2") {
  print "Plotting positions for 2 BSSs"
  plot \
  'spatial-reuse-positions.csv' i 0 with points ps 0 pt 13 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 with points ps 2 pt 13 lc rgb "red" title 'AP A', \
  'spatial-reuse-positions.csv' i 2 with points ps 2 pt 12 lc rgb "blue" title 'AP B', \
  'spatial-reuse-positions.csv' i 5 with points pt 28 lc rgb "red" title 'STA A', \
  'spatial-reuse-positions.csv' i 6 with points pt 27 lc rgb "blue" title 'STA B', \
  'spatial-reuse-positions.csv' i 1 using 1:2:3 with circles lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:4 with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:(-$4) with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:2:4 with circles lt 2 dt 2 lc rgb "red" title "A CSR", \
  'spatial-reuse-positions.csv' i 2 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "B CSR"
}

if (ARG1 == "3") {
  print "Plotting positions for 3 BSSs"
  plot \
  'spatial-reuse-positions.csv' i 0 with points ps 0 pt 13 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 with points ps 2 pt 13 lc rgb "red" title 'AP A', \
  'spatial-reuse-positions.csv' i 2 with points ps 2 pt 12 lc rgb "blue" title 'AP B', \
  'spatial-reuse-positions.csv' i 3 with points ps 2 pt 12 lc rgb "blue" title 'AP C', \
  'spatial-reuse-positions.csv' i 5 with points pt 28 lc rgb "red" title 'STA A', \
  'spatial-reuse-positions.csv' i 6 with points pt 27 lc rgb "blue" title 'STA B', \
  'spatial-reuse-positions.csv' i 7 with points pt 27 lc rgb "blue" title 'STA C', \
  'spatial-reuse-positions.csv' i 1 using 1:2:3 with circles lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:4 with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:(-$4) with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:2:4 with circles lt 2 dt 2 lc rgb "red" title "A CSR", \
  'spatial-reuse-positions.csv' i 2 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "B CSR", \
  'spatial-reuse-positions.csv' i 3 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "C CSR"
}

if (ARG1 == "4") {
  print "Plotting positions for 4 BSSs"
  plot \
  'spatial-reuse-positions.csv' i 0 with points ps 0 pt 13 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 with points ps 2 pt 13 lc rgb "red" title 'AP A', \
  'spatial-reuse-positions.csv' i 2 with points ps 2 pt 12 lc rgb "blue" title 'AP B', \
  'spatial-reuse-positions.csv' i 3 with points ps 2 pt 12 lc rgb "blue" title 'AP C', \
  'spatial-reuse-positions.csv' i 4 with points ps 2 pt 12 lc rgb "black" title 'AP D', \
  'spatial-reuse-positions.csv' i 5 with points pt 28 lc rgb "red" title 'STA A', \
  'spatial-reuse-positions.csv' i 6 with points pt 27 lc rgb "blue" title 'STA B', \
  'spatial-reuse-positions.csv' i 7 with points pt 27 lc rgb "blue" title 'STA C', \
  'spatial-reuse-positions.csv' i 8 with points pt 27 lc rgb "black" title 'STA D', \
  'spatial-reuse-positions.csv' i 1 using 1:2:3 with circles lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:2:3 with circles lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:4 with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:(-$4) with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:2:4 with circles lt 2 dt 2 lc rgb "red" title "A CSR", \
  'spatial-reuse-positions.csv' i 2 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "B CSR", \
  'spatial-reuse-positions.csv' i 3 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "C CSR", \
  'spatial-reuse-positions.csv' i 4 using 1:4 with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:(-$4) with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:2:4 with circles lt 2 dt 2 lc rgb "black" title "D CSR"
}
if (ARG1 == "5") {
  print "Plotting positions for 5 BSSs"
  plot \
  'spatial-reuse-positions.csv' i 0 with points ps 0 pt 13 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 with points ps 2 pt 13 lc rgb "red" title 'AP A', \
  'spatial-reuse-positions.csv' i 2 with points ps 2 pt 12 lc rgb "blue" title 'AP B', \
  'spatial-reuse-positions.csv' i 3 with points ps 2 pt 12 lc rgb "blue" title 'AP C', \
  'spatial-reuse-positions.csv' i 4 with points ps 2 pt 12 lc rgb "black" title 'AP D', \
  'spatial-reuse-positions.csv' i 5 with points ps 2 pt 12 lc rgb "black" title 'AP E', \
  'spatial-reuse-positions.csv' i 6 with points pt 28 lc rgb "red" title 'STA A', \
  'spatial-reuse-positions.csv' i 7 with points pt 27 lc rgb "blue" title 'STA B', \
  'spatial-reuse-positions.csv' i 8 with points pt 27 lc rgb "blue" title 'STA C', \
  'spatial-reuse-positions.csv' i 9 with points pt 27 lc rgb "black" title 'STA D', \
  'spatial-reuse-positions.csv' i 10 with points pt 27 lc rgb "black" title 'STA E', \
  'spatial-reuse-positions.csv' i 1 using 1:2:3 with circles lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:2:3 with circles lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 5 using 1:2:3 with circles lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:4 with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:(-$4) with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:2:4 with circles lt 2 dt 2 lc rgb "red" title "A CSR", \
  'spatial-reuse-positions.csv' i 2 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "B CSR", \
  'spatial-reuse-positions.csv' i 3 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "C CSR", \
  'spatial-reuse-positions.csv' i 4 using 1:4 with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:(-$4) with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:2:4 with circles lt 2 dt 2 lc rgb "black" title "D CSR"
  'spatial-reuse-positions.csv' i 5 using 1:4 with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 5 using 1:(-$4) with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 5 using 1:2:4 with circles lt 2 dt 2 lc rgb "black" title "E CSR"
}
if (ARG1 == "6") {
  print "Plotting positions for 6 BSSs"
  plot \
  'spatial-reuse-positions.csv' i 0 with points ps 0 pt 13 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 with points ps 2 pt 13 lc rgb "red" title 'AP A', \
  'spatial-reuse-positions.csv' i 2 with points ps 2 pt 12 lc rgb "blue" title 'AP B', \
  'spatial-reuse-positions.csv' i 3 with points ps 2 pt 12 lc rgb "blue" title 'AP C', \
  'spatial-reuse-positions.csv' i 4 with points ps 2 pt 12 lc rgb "black" title 'AP D', \
  'spatial-reuse-positions.csv' i 5 with points ps 2 pt 12 lc rgb "blue" title 'AP E', \
  'spatial-reuse-positions.csv' i 6 with points ps 2 pt 12 lc rgb "black" title 'AP F', \
  'spatial-reuse-positions.csv' i 7 with points pt 28 lc rgb "red" title 'STA A', \
  'spatial-reuse-positions.csv' i 8 with points pt 27 lc rgb "blue" title 'STA B', \
  'spatial-reuse-positions.csv' i 9 with points pt 27 lc rgb "blue" title 'STA C', \
  'spatial-reuse-positions.csv' i 10 with points pt 27 lc rgb "black" title 'STA D', \
  'spatial-reuse-positions.csv' i 11 with points pt 27 lc rgb "blue" title 'STA E', \
  'spatial-reuse-positions.csv' i 12 with points pt 27 lc rgb "black" title 'STA F', \
  'spatial-reuse-positions.csv' i 1 using 1:2:3 with circles lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:2:3 with circles lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 5 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 6 using 1:2:3 with circles lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:4 with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:(-$4) with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:2:4 with circles lt 2 dt 2 lc rgb "red" title "A CSR", \
  'spatial-reuse-positions.csv' i 2 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "B CSR", \
  'spatial-reuse-positions.csv' i 3 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "C CSR", \
  'spatial-reuse-positions.csv' i 4 using 1:4 with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:(-$4) with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:2:4 with circles lt 2 dt 2 lc rgb "black" title "D CSR", \
  'spatial-reuse-positions.csv' i 5 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 5 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 5 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "E CSR", \
  'spatial-reuse-positions.csv' i 6 using 1:4 with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 6 using 1:(-$4) with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 6 using 1:2:4 with circles lt 2 dt 2 lc rgb "black" title "F CSR", \

}

if (ARG1 == "7") {
  print "Plotting positions for 7 BSSs"
  plot \
  'spatial-reuse-positions.csv' i 0 with points ps 0 pt 13 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 with points ps 2 pt 13 lc rgb "red" title 'AP A', \
  'spatial-reuse-positions.csv' i 2 with points ps 2 pt 12 lc rgb "blue" title 'AP B', \
  'spatial-reuse-positions.csv' i 3 with points ps 2 pt 12 lc rgb "blue" title 'AP C', \
  'spatial-reuse-positions.csv' i 4 with points ps 2 pt 12 lc rgb "black" title 'AP D', \
  'spatial-reuse-positions.csv' i 5 with points ps 2 pt 12 lc rgb "blue" title 'AP E', \
  'spatial-reuse-positions.csv' i 6 with points ps 2 pt 12 lc rgb "black" title 'AP F', \
  'spatial-reuse-positions.csv' i 7 with points ps 2 pt 12 lc rgb "black" title 'AP G', \
  'spatial-reuse-positions.csv' i 8 with points pt 28 lc rgb "red" title 'STA A', \
  'spatial-reuse-positions.csv' i 9 with points pt 27 lc rgb "blue" title 'STA B', \
  'spatial-reuse-positions.csv' i 10 with points pt 27 lc rgb "blue" title 'STA C', \
  'spatial-reuse-positions.csv' i 11 with points pt 27 lc rgb "black" title 'STA D', \
  'spatial-reuse-positions.csv' i 12 with points pt 27 lc rgb "blue" title 'STA E', \
  'spatial-reuse-positions.csv' i 13 with points pt 27 lc rgb "black" title 'STA F', \
  'spatial-reuse-positions.csv' i 14 with points pt 27 lc rgb "black" title 'STA G', \
  'spatial-reuse-positions.csv' i 1 using 1:2:3 with circles lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:2:3 with circles lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 5 using 1:2:3 with circles lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 6 using 1:2:3 with circles lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 7 using 1:2:3 with circles lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:4 with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:(-$4) with points ps 0 pt -1 lc rgb "red" notitle, \
  'spatial-reuse-positions.csv' i 1 using 1:2:4 with circles lt 2 dt 2 lc rgb "red" title "A CSR", \
  'spatial-reuse-positions.csv' i 2 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 2 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "B CSR", \
  'spatial-reuse-positions.csv' i 3 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 3 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "C CSR", \
  'spatial-reuse-positions.csv' i 4 using 1:4 with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:(-$4) with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 4 using 1:2:4 with circles lt 2 dt 2 lc rgb "black" title "D CSR", \
  'spatial-reuse-positions.csv' i 5 using 1:4 with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 5 using 1:(-$4) with points ps 0 pt -1 lc rgb "blue" notitle, \
  'spatial-reuse-positions.csv' i 5 using 1:2:4 with circles lt 2 dt 2 lc rgb "blue" title "E CSR", \
  'spatial-reuse-positions.csv' i 6 using 1:4 with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 6 using 1:(-$4) with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 6 using 1:2:4 with circles lt 2 dt 2 lc rgb "black" title "F CSR", \
  'spatial-reuse-positions.csv' i 7 using 1:4 with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 7 using 1:(-$4) with points ps 0 pt -1 lc rgb "black" notitle, \
  'spatial-reuse-positions.csv' i 7 using 1:2:4 with circles lt 2 dt 2 lc rgb "black" title "G CSR"
}
quit
