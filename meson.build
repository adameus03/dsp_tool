project('signal_plotter_combiner', 'c')
gtkdep = dependency('gtk+-3.0')
executable('signal_plotter_combiner', ['main.c', 'controller.c', 'model/gnuplot.c'], dependencies : gtkdep, export_dynamic: true) #https://stackoverflow.com/questions/49545752/error-did-you-compile-with-rdynamic-seen-only-when-application-installed-usi