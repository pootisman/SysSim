set terminal postscript eps enhanced
set output "First stage R(t).eps"

set ylabel "R(t)"
set xlabel "t"

plot "histogram_furst.dat" u 1:2 with lines title "R(t)",\
     0.5*exp(-0.5*x) + 0.5*exp(-0.25*x) with lines title "R_{th}(t)"

set output "First stage a(t).eps"
set ylabel "{/Symbol l}"

plot "histogram_furst.dat" u 1:3 with lines title "{/Symbol l}(t)",\
     (0.5*exp(-0.5*x)*0.5 + 0.25*exp(-0.25*x)*0.5)/(exp(-0.5*x)*0.5 + exp(-0.25*x)*0.5) with lines title "{/Symbol l}_{th}(t)"

#set ylabel "Devices Alive"

#set logscale y
#plot "histogram_furst.dat" u 1:4 with lines title "Alive."
#set nologscale y

set output "Second stage R(t).eps"

set ylabel "R(t)"

plot "histogram_secund.dat" u 1:2 with lines title "R(t)",\
     exp(-0.75*x) with lines title "R_{th}(t)"

set output "Second stage a(t).eps"
set ylabel "{/Symbol l}"

plot "histogram_secund.dat" u 1:3 with lines title "{/Symbol l}(t)",\
     0.75*exp(-0.75*x)/exp(-0.75*x) with lines title "{/Symbol l}_{th}(t)"

#set ylabel "Devices Alive"

#set logscale y
#plot "histogram_secund.dat" u 1:4 with lines title "Alive."
#set nologscale y

set output "Third stage R(t).eps"

set ylabel "R(t)"

plot "histogram_thurd.dat" u 1:2 with lines title "R(t)",\
     exp(-0.5*x)*(0.5*x + 1) with lines title "R_{th}(t)"

set output "Third stage a(t).eps"
set ylabel "{/Symbol l}"

plot "histogram_thurd.dat" u 1:3 with lines title "{/Symbol l}(t)",\
     -(-0.5*exp(-0.5*x)*(0.5*x + 1) + exp(-0.5*x)*0.5)/(exp(-0.5*x)*(0.5*x + 1)) with lines title "{/Symbol l}_{th}(t)"

#set ylabel "Devices Alive"

#set logscale y
#plot "histogram_thurd.dat" u 1:4 with lines title "Alive."
#set nologscale y

set output "Recoverable.eps"

set ylabel "K_r"
plot "plotData.dat" u 1:2 with lines title "K_r",\
     0.88 with lines title "K_{rth}"

set output "Recoverable final.eps"
plot "plotData_multi.dat" u 1:2 with lines title "K_r",\
     0.94613 with lines title "K_{low}",\
     0.9725 with lines title "K_{hi}"
