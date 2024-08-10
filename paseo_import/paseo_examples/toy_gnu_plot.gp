# Mac support
set term qt font "Arial 12"
# Nice output
set terminal svg size 800,600
set output 'steps_per_hour.svg'

# Set the title of the plot
set title "Steps per Hour"

# Set the x-axis label
set xlabel "Time (Unix Time)"

# Set the y-axis label
set ylabel "Steps"

# Set the format for the x-axis to display as date and time
set xdata time
set timefmt "%s"
set format x "%Y-%m-%d\n%H:%M:%S"

# Set the grid
set grid

# Plot the data from the TSV file
plot 'paseo_export_1723308962.tsv' using 1:2 with lines title "Steps"


