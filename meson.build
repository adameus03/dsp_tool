project('signal_plotter_combiner', 'c')

#add_project_arguments('-g', language: 'c')
#add_project_arguments('-O2', language: 'c')

gtkdep = dependency('gtk+-3.0')
#mdep = dependency('m', method : 'build-in') #https://stackoverflow.com/questions/60927711/how-to-set-meson-link-dependency-for-buildin-lib-as-lm-lrt - seems not to be working
cc = meson.get_compiler('c')
mdep = cc.find_library('m', required : false)
executable('signal_plotter_combiner', [
    'main.c', 
    'controller.c', 
    'model/gnuplot.c', 
    'model/generator.c',
    'model/signal.c',
    'model/transform.c',
    'model/combiner.c',
    'model/aggregator.c',
    'model/__gcall.h',
    'model/signal_fio.c'
], dependencies : [gtkdep, mdep], export_dynamic: true) #https://stackoverflow.com/questions/49545752/error-did-you-compile-with-rdynamic-seen-only-when-application-installed-usi