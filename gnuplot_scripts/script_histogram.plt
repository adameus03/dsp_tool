#Adapted from https://gnuplot-surprising.blogspot.com/2011/09/statistic-analysis-and-histogram.html

# Required args:
#   n - number of intervals
#   min - min value
#   max - max value
#   infile - input file path
#   outfile - output file path
#   plottitle - rendered plot title

# Example gnuplot call 
# gnuplot -e "max=1." -e "min=-1." -e "n=10" -e "outfile='foo.png'" -e "infile='data.txt'" -e "plottitle='Signal A histogram'" script.plt


reset
width=(max-min)/n #interval width

#function used to map a value to the intervals
hist(x,width)=width*floor(x/width)+width/2.0

set term png #output terminal and file
set output outfile
set xrange [min:max]
set yrange [0:]
#to put an empty boundary around the
#data inside an autoscaled graph.
set offset graph 0.05,0.05,0.05,0.0
set xtics min,(max-min)/5,max
set boxwidth width*0.9
set style fill solid 0.5 #fillstyle
set tics out nomirror
set title plottitle #
set xlabel "Signal value"
set ylabel "Value frequency"
#count and plot
plot infile u (hist($2,width)):(1.0) smooth freq w boxes lc rgb"green" notitle