# Map2Prop

## Introduction

Map2Prop is a tool for converting .map, .rmf and .jmf files as well as .obj files exported from J.A.C.K and .ol prefab libraries to GoldSrc .smd file that can then be compiled into a GoldSrc format studio model without the hassle of using an 3D editor.

It can also be seamlessly integrated with the map compilation process, essentially running as its own compilation tool that'll convert, compile, and replace the brushwork with model props automatically.

For more detailed documentation you can check out [Map2Prop Docs](https://erty-gamedev.github.io/Docs-Map2Prop/).

## Requirements

To use the auto-compile feature one must add a valid reference to a [Sven Co-op studiomdl.exe](http://www.the303.org/backups/sven_studiomdl_2019.rar) either in [config.ini](#configini) (recommended) or using the commandline option (`-m`, `--studiomdl`).

#### Why is the Sven Co-op studiomdl.exe required for compilation?

The reason for requiring the Sven Co-op studiomdl.exe for compiling these models is because of how map textures work, i.e. they may tile or otherwise extend beyond the UV bounds. Legacy studiomdl.exe compilers will clamp UV coordinates which is no good for this. Additionally, legacy compilers cannot handle CHROME textures that aren't 64x64. Don't worry, the compiled model will still work perfectly fine in vanilla Half-Life.

## How To Use

Either download and unzip the latest archive from [Releases](https://github.com/Erty-Gamedev/Map2Prop/releases/latest), or clone this repo and build the project yourself (see [Building](#building)) to create your own executable.

After that open config.ini and ensure the paths to the Steam installation and studiomdl.exe are set up correctly.

### Drag & Drop

A simple way to use Map2Prop is to place the prop(s) to be converted in their own map and drag that map onto the executable.

All objects will be merged with the worldspawn model unless part of a func_map2prop entity with the `own_model` key set to `1`, which will be placed in their own models.

### Map Compile

By adding the `map2prop.fgd` to the map editor's game configuration one can tie brushes to the `func_map2prop` entity and add the Map2Prop executable to the compilation process (for example Hammer/J.A.C.K's Run Map Expert mode) before/above the CSG compiler and in arguments put `"$path/$file" --mapcompile`.

You may also include the file extension of the project file to force Map2Prop to read directly from that file, as both RMF and JMF provides greater floating point precision than the MAP format. For example: `"$path/$file.rmf" --mapcompile`

**Important:** Ensure *Use Process Window* or *Wait for Termination* (or similar) is checked to make sure Map2Prop finishes before the rest of the process continues.

### Commandline Interface

For advanced usage one might run Map2Prop through a shell or command prompt.

To get started with the CLI one can either run Map2Prop with no arguments or call it with `--help` to get a list of available arguments.

### Textures

Map2Prop needs the texture files that are used in the prop to create the models. It will look for the raw BMP textures in both the input file's directory and the output directory, as well as in WAD archives in these directories and the configured game config's mod folder and automatically extract these.<br />
Additionally one can feed a text file containing paths to WAD archives (one per line) to the commandline argument (`-w`, `--wad_list`) or fill out a `wad list` in [config.ini](#configini).

The easiest way is to set the `steam directory` and the relevant `game` and `mod` in [config.ini](#configini) and Map2Prop will take care of finding and extracting the textures for you.

## config.ini

The config file `config.ini` allows one to adjust various defaults and set up paths to the model compiler and game directories. Each separate game configuration section (preceeded by the section's name in square brackets, e.g. `[halflife]`) will override the default settings when that configuration is active. The active game configuration can either be set in the default's `game config` key, or by using the commandline option (`-g`, `--game_config`).

For example, when using the map compile feature one can add the `--game_config my_mod` to ensure the models end up in */Half-Life/my_mod/models/*

## Building

The repository provides CMake files for cross-platform building and has been tested on Windows 10 (MSVC v143) and Ubuntu 24.04 (GCC v13.3.0).

From the project root, create a build/ directory and cd into it:

```shell
mkdir build
cd build
```

From the build/ directory, run CMake on the parent directory to generate the build system:

```shell
cmake ../
```

*You may also specify the build system generator with the `-G <generator-name>` flag. Use `cmake --help` to get a list of available generators on your system.*

> To set the build configuration, use the `cmake -DCMAKE_BUILD_TYPE=<configuration>` command where `<configuration>` is either `Debug` or `Release`.

Finally build the project by running with the `--build` flag:

```shell
cmake --build .
```

The executable will now be found in the bin/<configuration> directory (by default this is bin/debug).

## Bug Reports and Feature Suggestions

Take a look at the [Issues](https://github.com/Erty-Gamedev/Map2Prop/issues) page and make sure the bug/feature hasn't been already been posted.

If the bug/feature is new, feel free to open a new issue. Be as detailed and specific as you can be.<br />
If it's a bug report, please include the steps for reproducing the issue. Any log files (from /logs) or input map files will be very helpful in investigating the bug.

## Contributing
Please see [CONTRIBUTING.md](./.github/CONTRIBUTING.md).

## Special Thanks

Many thanks goes out to these great people who have provided *lots* of help, feedback, advice and more throughout the development of this application:
* Captain P
* Penguinboy
* Kimilil
* AdamBean
* generic
* RaptorSKA

### Alpha Testers

I would also like to thank the kind and patient people who helped me test this application and provide useful feedback and suggestions during the first version's alpha stage:
* SV BOY
* TheMadCarrot
* Descen
* Kimilil
