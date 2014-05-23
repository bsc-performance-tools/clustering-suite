set datafile separator ","
set fontpath "/home/jgonzale/.fonts"
set terminal pngcairo enhanced font "Lucida Grande" size 640,480
set output "mainmem_vs_l2_plot.png"

set datafile separator ","

set xlabel "L2 Cache Accesses" font ", 22"
set ylabel "Main Memory Lines" font ", 22"

set key bottom font ", 16"
set grid
set logscale xy

set size square

set format x "%.0e"
set format y "%.0e"
set xtics rotate

plot 'GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster1.csv' using 27:26  w points ps 2 lw 2 lt rgbcolor "#00ff00" title 'Cluster 1',\
'GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster2.csv' using 27:26 w points ps 2 lw 2 lt rgbcolor "#ffff00" title 'Cluster 2',\
'GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster3.csv'using 27:26 w points ps 2 lw 2 lt rgbcolor "#eb0000" title 'Cluster 3',\
'GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster4.csv' using 27:26 w points ps 2 lw 2 lt rgbcolor "#00a200" title 'Cluster 4'
pause -1 "Hit return to continue..."
