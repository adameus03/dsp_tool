This is a C project aiming to assist the user with DSP experiments, mainly real and complex signal analysis, comparing signals, combining signals, signal filtering using finite impulse response filters and performing frequency-domain transforms on digital signals. The project uses the Meson build system and the GTK3 libraries, so make sure to have the neccessary libraries available on your system. The project is developed on a GNU/Linux based system. Integration with other platforms wasn't tested.

### Setup the project
`dsp_tool $ meson setup builddir`

### Building the project
`dsp_tool/builddir $ meson compile`

### Running tests
`dsp_tool/builddir $ meson test`

### Running project
`dsp_tool $ builddir/dsp_tool`

### Contributing
If you are interested in the project and wish to develop a part of this software, feel free to check the currently open issues and submit any pull requests for this repository. There's a few tasks denoted with [TODO]
 markers in the code, which are not listed in the issues, however they are necessary for further development of the project.
