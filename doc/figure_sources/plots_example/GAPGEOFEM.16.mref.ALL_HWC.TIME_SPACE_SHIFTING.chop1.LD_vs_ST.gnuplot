set fontpath "/home/jgonzale/.fonts"
set terminal pdf enhanced color font "Lucida Grande" size 4,3
set output "stores_vs_loads_plot.pdf"

set datafile separator ","

set xlabel "Loads" font ", 10"
set ylabel "Stores" font ", 10" 

set key bottom font ", 8"
set grid
set logscale xy

set format x "%.0e"
set format y "%.0e"
set xtics rotate

plot 'GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster1.csv' using 19:20  w points ps 0.65 lw 2 lt rgbcolor "#00ff00" title 'Cluster 1',\
'GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster2.csv' using 19:20 w points ps 0.65 lw 2 lt rgbcolor "#ffff00" title 'Cluster 2',\
'GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster3.csv'using 19:20 w points ps 0.65 lw 2 lt rgbcolor "#eb0000" title 'Cluster 3',\
'GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster4.csv' using 19:20 w points ps 0.65 lw 2 lt rgbcolor "#00a200" title 'Cluster 4'
pause -1 "Hit return to continue..."
