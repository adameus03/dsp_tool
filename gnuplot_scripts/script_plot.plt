# Required args:
#   infile - input file path
#   outfile - output file path
#   plottitle - rendered plot title
#   sizex - output image width
#   sizey - output image height
#   domainUnitsLabel - measuring unit description label for the domain set 
#   codomainUnitsLabel - measuring unit description label for the codomain (values) set

# Example gnuplot call 
# gnuplot -e "outfile='foo.png'" -e "infile='data.txt'" -e "plottitle='Signal A histogram'" -e "sizex=800" -e "sizey=640" -e "domainUnitsLabel='t [s]'" -e "codomainUnitsLabel='A [V]'" script.plt


_font = "Times-New-Roman,20"
set terminal pngcairo font _font size sizex,sizey #
set output outfile #
set encoding utf8

set title plottitle #
set xlabel domainUnitsLabel font _font
set ylabel codomainUnitsLabel font _font

set style line 1 dashtype 1 linecolor rgb "#000000" linewidth 1.5 pointtype 7 pointsize 0.5

plot infile notitle with points linestyle 1 axes x1y1 #
set output