project('signal_plotter_combiner', 'c')

project_test_files = [
  'tests/transform_tests.c',
]



#add_project_arguments('-g', language: 'c')
#add_project_arguments('-O2', language: 'c')

gtk3_dep = dependency('gtk+-3.0')
#mdep = dependency('m', method : 'build-in') #https://stackoverflow.com/questions/60927711/how-to-set-meson-link-dependency-for-buildin-lib-as-lm-lrt - seems not to be working
cc = meson.get_compiler('c')
add_project_arguments('-g', language: 'c')
m_dep = cc.find_library('m', required : false)
src = [
    'main.c', 
    'controller.c', 
    'controller_timeshift.c',
    'controller_fir.c',
    'gui_tweaks.c',
    'model/gnuplot.c', 
    'model/generator.c',
    'model/signal.c',
    'model/transform.c',
    'model/combiner.c',
    'model/aggregator.c',
    'model/fir.c',
    'model/__gcall.h',
    'model/signal_fio.c',
    'model/converters/adc.c',
    'model/converters/dac.c',
    'model/measures/similarity.c'

]
executable('signal_plotter_combiner', src, 
    dependencies : [gtk3_dep, m_dep], export_dynamic: true) #https://stackoverflow.com/questions/49545752/error-did-you-compile-with-rdynamic-seen-only-when-application-installed-usi

lib_src = [
    'model/gnuplot.c', 
    'model/generator.c',
    'model/signal.c',
    'model/transform.c',
    'model/combiner.c',
    'model/aggregator.c',
    'model/fir.c',
    'model/__gcall.h',
    'model/signal_fio.c',
    'model/converters/adc.c',
    'model/converters/dac.c',
    'model/measures/similarity.c'
]

lib = static_library('signal_plotter_combiner_lib', lib_src, dependencies : [m_dep])

subdir('tests')

# test('all_tests',
#     executable(
#         'run_tests',
#         files(project_test_files),
#         dependencies : [test_dep],
#         install : false
#     )
# )