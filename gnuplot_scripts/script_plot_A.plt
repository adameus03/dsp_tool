_font = "Times-New-Roman,20"
set terminal pngcairo font _font size 800,640
set output 'out.png'
set encoding utf8

set title "Signal A"
set xlabel "t [s]" font _font
set ylabel "A [V]" font _font

set style line 1 dashtype 1 linecolor rgb "#000000" linewidth 1.5 pointtype 7 pointsize 0.1

plot './data.txt' notitle with points linestyle 1 axes x1y1
set output