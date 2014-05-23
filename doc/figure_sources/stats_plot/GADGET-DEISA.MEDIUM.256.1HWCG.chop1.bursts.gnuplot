set fontpath "/home/jgonzale/.fonts"
set terminal pdf enhanced color font "Lucida Grande"
set output "stats_plot.pdf"

#set title "Histogram of bursts" 

set key bottom outside center  font ",8"

set ylabel "Number of bursts" font ",10"
set y2label "% of computation time" font ",10"

set ytics nomirror
set ytics format  "%2.0t{/Symbol \264}10^{%L}"


set y2tics 
set y2range [0:100]
set y2tics format "%3.0f"

set xtics ("1 {/Symbol m}s" 0, "10 {/Symbol m}s" 2, "100 {/Symbol m}s" 4, "1 ms" 6, "10 ms" 8, "100 ms" 10, "1s" 12, "10 s" 14)
set xtic rotate by -45 scale 2 font ",8"
set offsets 2, 2, 0, 0

set style line 2 linecolor rgb "#32CD32"

#set boxwidth 0.5 absolute
set style fill  solid 1.00 border -1
plot 'GADGET-DEISA.MEDIUM.256.1HWCG.chop1.bursts.dat' using 0:1 t "Number of bursts" w lines, 'GADGET-DEISA.MEDIUM.256.1HWCG.chop1.bursts.dat' using 0:2 axes x1y2 ls 2 t "% of computing time" with boxes
pause -1 "Press return to continue..."
